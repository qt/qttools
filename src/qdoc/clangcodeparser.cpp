// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "clangcodeparser.h"

#include "access.h"
#include "classnode.h"
#include "codechunk.h"
#include "config.h"
#include "enumnode.h"
#include "functionnode.h"
#include "namespacenode.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "typedefnode.h"
#include "utilities.h"
#include "variablenode.h"

#include <QtCore/qdebug.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qfile.h>
#include <QtCore/qscopedvaluerollback.h>
#include <QtCore/qtemporarydir.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qvarlengtharray.h>

#include <clang-c/Index.h>

#include <clang/AST/Decl.h>
#include <clang/AST/DeclFriend.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Lex/Lexer.h>
#include <llvm/Support/Casting.h>

#include <cstdio>

QT_BEGIN_NAMESPACE

const QStringList ClangCodeParser::accepted_header_file_extensions{
    "ch", "h", "h++", "hh", "hpp", "hxx"
};

// We're printing diagnostics in ClangCodeParser::printDiagnostics,
// so avoid clang itself printing them.
static const auto kClangDontDisplayDiagnostics = 0;

static CXTranslationUnit_Flags flags_ = static_cast<CXTranslationUnit_Flags>(0);
static CXIndex index_ = nullptr;

QByteArray ClangCodeParser::s_fn;
constexpr const char fnDummyFileName[] = "/fn_dummyfile.cpp";

#ifndef QT_NO_DEBUG_STREAM
template<class T>
static QDebug operator<<(QDebug debug, const std::vector<T> &v)
{
    QDebugStateSaver saver(debug);
    debug.noquote();
    debug.nospace();
    const size_t size = v.size();
    debug << "std::vector<>[" << size << "](";
    for (size_t i = 0; i < size; ++i) {
        if (i)
            debug << ", ";
        debug << v[i];
    }
    debug << ')';
    return debug;
}
#endif // !QT_NO_DEBUG_STREAM

/*!
   Call clang_visitChildren on the given cursor with the lambda as a callback
   T can be any functor that is callable with a CXCursor parameter and returns a CXChildVisitResult
   (in other word compatible with function<CXChildVisitResult(CXCursor)>
 */
template<typename T>
bool visitChildrenLambda(CXCursor cursor, T &&lambda)
{
    CXCursorVisitor visitor = [](CXCursor c, CXCursor,
                                 CXClientData client_data) -> CXChildVisitResult {
        return (*static_cast<T *>(client_data))(c);
    };
    return clang_visitChildren(cursor, visitor, &lambda);
}

/*!
    convert a CXString to a QString, and dispose the CXString
 */
static QString fromCXString(CXString &&string)
{
    QString ret = QString::fromUtf8(clang_getCString(string));
    clang_disposeString(string);
    return ret;
}

static QString templateDecl(CXCursor cursor);

/*!
    Returns a list of template parameters at \a cursor.
*/
static QStringList getTemplateParameters(CXCursor cursor)
{
    QStringList parameters;
    visitChildrenLambda(cursor, [&parameters](CXCursor cur) {
        QString name = fromCXString(clang_getCursorSpelling(cur));
        QString type;

        switch (clang_getCursorKind(cur)) {
        case CXCursor_TemplateTypeParameter:
            type = QStringLiteral("typename");
            break;
        case CXCursor_NonTypeTemplateParameter:
            type = fromCXString(clang_getTypeSpelling(clang_getCursorType(cur)));
            // Hack: Omit QtPrivate template parameters from public documentation
            if (type.startsWith(QLatin1String("QtPrivate")))
                return CXChildVisit_Continue;
            break;
        case CXCursor_TemplateTemplateParameter:
            type = templateDecl(cur) + QLatin1String(" class");
            break;
        default:
            return CXChildVisit_Continue;
        }

        if (!name.isEmpty())
            name.prepend(QLatin1Char(' '));

        parameters << type + name;
        return CXChildVisit_Continue;
    });

    return parameters;
}

/*!
   Gets the template declaration at specified \a cursor.
 */
static QString templateDecl(CXCursor cursor)
{
    QStringList params = getTemplateParameters(cursor);
    return QLatin1String("template <") + params.join(QLatin1String(", ")) + QLatin1Char('>');
}

/*!
    convert a CXSourceLocation to a qdoc Location
 */
static Location fromCXSourceLocation(CXSourceLocation location)
{
    unsigned int line, column;
    CXString file;
    clang_getPresumedLocation(location, &file, &line, &column);
    Location l(fromCXString(std::move(file)));
    l.setColumnNo(column);
    l.setLineNo(line);
    return l;
}

/*!
    convert a CX_CXXAccessSpecifier to Node::Access
 */
static Access fromCX_CXXAccessSpecifier(CX_CXXAccessSpecifier spec)
{
    switch (spec) {
    case CX_CXXPrivate:
        return Access::Private;
    case CX_CXXProtected:
        return Access::Protected;
    case CX_CXXPublic:
        return Access::Public;
    default:
        return Access::Public;
    }
}

/*!
   Returns the spelling in the file for a source range
 */

struct FileCacheEntry
{
    QByteArray fileName;
    QByteArray content;
};

static inline QString fromCache(const QByteArray &cache,
                                unsigned int offset1, unsigned int offset2)
{
    return QString::fromUtf8(cache.mid(offset1, offset2 - offset1));
}

static QString readFile(CXFile cxFile, unsigned int offset1, unsigned int offset2)
{
    using FileCache = QList<FileCacheEntry>;
    static FileCache cache;

    CXString cxFileName = clang_getFileName(cxFile);
    const QByteArray fileName = clang_getCString(cxFileName);
    clang_disposeString(cxFileName);

    for (const auto &entry : std::as_const(cache)) {
        if (fileName == entry.fileName)
            return fromCache(entry.content, offset1, offset2);
    }

    // "fn_dummyfile.cpp" comes with varying cxFile values
    if (fileName == fnDummyFileName)
        return fromCache(ClangCodeParser::fn(), offset1, offset2);

    QFile file(QString::fromUtf8(fileName));
    if (file.open(QIODeviceBase::ReadOnly)) { // binary to match clang offsets
        FileCacheEntry entry{fileName, file.readAll()};
        cache.prepend(entry);
        while (cache.size() > 5)
            cache.removeLast();
        return fromCache(entry.content, offset1, offset2);
    }
    return {};
}

static QString getSpelling(CXSourceRange range)
{
    auto start = clang_getRangeStart(range);
    auto end = clang_getRangeEnd(range);
    CXFile file1, file2;
    unsigned int offset1, offset2;
    clang_getFileLocation(start, &file1, nullptr, nullptr, &offset1);
    clang_getFileLocation(end, &file2, nullptr, nullptr, &offset2);

    if (file1 != file2 || offset2 <= offset1)
        return QString();

    return readFile(file1, offset1, offset2);
}

/*!
  Returns the function name from a given cursor representing a
  function declaration. This is usually clang_getCursorSpelling, but
  not for the conversion function in which case it is a bit more complicated
 */
QString functionName(CXCursor cursor)
{
    if (clang_getCursorKind(cursor) == CXCursor_ConversionFunction) {
        // For a CXCursor_ConversionFunction we don't want the spelling which would be something
        // like "operator type-parameter-0-0" or "operator unsigned int". we want the actual name as
        // spelled;
        QString type = fromCXString(clang_getTypeSpelling(clang_getCursorResultType(cursor)));
        if (type.isEmpty())
            return fromCXString(clang_getCursorSpelling(cursor));
        return QLatin1String("operator ") + type;
    }

    QString name = fromCXString(clang_getCursorSpelling(cursor));

    // Remove template stuff from constructor and destructor but not from operator<
    auto ltLoc = name.indexOf('<');
    if (ltLoc > 0 && !name.startsWith("operator<"))
        name = name.left(ltLoc);
    return name;
}

/*!
  Reconstruct the qualified path name of a function that is
  being overridden.
 */
static QString reconstructQualifiedPathForCursor(CXCursor cur)
{
    QString path;
    auto kind = clang_getCursorKind(cur);
    while (!clang_isInvalid(kind) && kind != CXCursor_TranslationUnit) {
        switch (kind) {
        case CXCursor_Namespace:
        case CXCursor_StructDecl:
        case CXCursor_ClassDecl:
        case CXCursor_UnionDecl:
        case CXCursor_ClassTemplate:
            path.prepend("::");
            path.prepend(fromCXString(clang_getCursorSpelling(cur)));
            break;
        case CXCursor_FunctionDecl:
        case CXCursor_FunctionTemplate:
        case CXCursor_CXXMethod:
        case CXCursor_Constructor:
        case CXCursor_Destructor:
        case CXCursor_ConversionFunction:
            path = functionName(cur);
            break;
        default:
            break;
        }
        cur = clang_getCursorSemanticParent(cur);
        kind = clang_getCursorKind(cur);
    }
    return path;
}

/*!
  Find the node from the QDocDatabase \a qdb that corrseponds to the declaration
  represented by the cursor \a cur, if it exists.
 */
static Node *findNodeForCursor(QDocDatabase *qdb, CXCursor cur)
{
    auto kind = clang_getCursorKind(cur);
    if (clang_isInvalid(kind))
        return nullptr;
    if (kind == CXCursor_TranslationUnit)
        return qdb->primaryTreeRoot();

    Node *p = findNodeForCursor(qdb, clang_getCursorSemanticParent(cur));
    if (p == nullptr)
        return nullptr;
    if (!p->isAggregate())
        return nullptr;
    auto parent = static_cast<Aggregate *>(p);

    QString name = fromCXString(clang_getCursorSpelling(cur));
    switch (kind) {
    case CXCursor_Namespace:
        return parent->findNonfunctionChild(name, &Node::isNamespace);
    case CXCursor_StructDecl:
    case CXCursor_ClassDecl:
    case CXCursor_UnionDecl:
    case CXCursor_ClassTemplate:
        return parent->findNonfunctionChild(name, &Node::isClassNode);
    case CXCursor_FunctionDecl:
    case CXCursor_FunctionTemplate:
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_ConversionFunction: {
        NodeVector candidates;
        parent->findChildren(functionName(cur), candidates);
        if (candidates.isEmpty())
            return nullptr;
        CXType funcType = clang_getCursorType(cur);
        auto numArg = clang_getNumArgTypes(funcType);
        bool isVariadic = clang_isFunctionTypeVariadic(funcType);
        QVarLengthArray<QString, 20> args;
        for (Node *candidate : std::as_const(candidates)) {
            if (!candidate->isFunction(Node::CPP))
                continue;
            auto fn = static_cast<FunctionNode *>(candidate);
            const Parameters &parameters = fn->parameters();
            if (parameters.count() != numArg + isVariadic)
                continue;
            if (fn->isConst() != bool(clang_CXXMethod_isConst(cur)))
                continue;
            if (isVariadic && parameters.last().type() != QLatin1String("..."))
                continue;
            if (fn->isRef() != (clang_Type_getCXXRefQualifier(funcType) == CXRefQualifier_LValue))
                continue;
            if (fn->isRefRef() != (clang_Type_getCXXRefQualifier(funcType) == CXRefQualifier_RValue))
                continue;

            bool different = false;
            for (int i = 0; i < numArg; ++i) {
                CXType argType = clang_getArgType(funcType, i);
                if (args.size() <= i)
                    args.append(fromCXString(clang_getTypeSpelling(argType)));
                QString recordedType = parameters.at(i).type();
                QString typeSpelling = args.at(i);
                auto p = parent;
                while (p && recordedType != typeSpelling) {
                    QString parentScope = p->name() + QLatin1String("::");
                    recordedType.remove(parentScope);
                    typeSpelling.remove(parentScope);
                    p = p->parent();
                }
                different = recordedType != typeSpelling;

                // Retry with a canonical type spelling
                if (different && (argType.kind == CXType_Typedef || argType.kind == CXType_Elaborated)) {
                    QStringView canonicalType = parameters.at(i).canonicalType();
                    if (!canonicalType.isEmpty()) {
                        different = canonicalType !=
                            fromCXString(clang_getTypeSpelling(clang_getCanonicalType(argType)));
                    }
                }
                if (different) {
                    break;
                }
            }
            if (!different)
                return fn;
        }
        return nullptr;
    }
    case CXCursor_EnumDecl:
        return parent->findNonfunctionChild(name, &Node::isEnumType);
    case CXCursor_FieldDecl:
    case CXCursor_VarDecl:
        return parent->findNonfunctionChild(name, &Node::isVariable);
    case CXCursor_TypedefDecl:
        return parent->findNonfunctionChild(name, &Node::isTypedef);
    default:
        return nullptr;
    }
}

static void setOverridesForFunction(FunctionNode *fn, CXCursor cursor)
{
    CXCursor *overridden;
    unsigned int numOverridden = 0;
    clang_getOverriddenCursors(cursor, &overridden, &numOverridden);
    for (uint i = 0; i < numOverridden; ++i) {
        QString path = reconstructQualifiedPathForCursor(overridden[i]);
        if (!path.isEmpty()) {
            fn->setOverride(true);
            fn->setOverridesThis(path);
            break;
        }
    }
    clang_disposeOverriddenCursors(overridden);
}

class ClangVisitor
{
public:
    ClangVisitor(QDocDatabase *qdb, const QMultiHash<QString, QString> &allHeaders)
        : qdb_(qdb), parent_(qdb->primaryTreeRoot()), allHeaders_(allHeaders)
    {
    }

    QDocDatabase *qdocDB() { return qdb_; }

    CXChildVisitResult visitChildren(CXCursor cursor)
    {
        auto ret = visitChildrenLambda(cursor, [&](CXCursor cur) {
            auto loc = clang_getCursorLocation(cur);
            if (clang_Location_isFromMainFile(loc))
                return visitSource(cur, loc);
            CXFile file;
            clang_getFileLocation(loc, &file, nullptr, nullptr, nullptr);
            bool isInteresting = false;
            auto it = isInterestingCache_.find(file);
            if (it != isInterestingCache_.end()) {
                isInteresting = *it;
            } else {
                QFileInfo fi(fromCXString(clang_getFileName(file)));
                // Match by file name in case of PCH/installed headers
                isInteresting = allHeaders_.contains(fi.fileName());
                isInterestingCache_[file] = isInteresting;
            }
            if (isInteresting) {
                return visitHeader(cur, loc);
            }

            return CXChildVisit_Continue;
        });
        return ret ? CXChildVisit_Break : CXChildVisit_Continue;
    }

    /*
      Not sure about all the possibilities, when the cursor
      location is not in the main file.
     */
    CXChildVisitResult visitFnArg(CXCursor cursor, Node **fnNode, bool &ignoreSignature)
    {
        auto ret = visitChildrenLambda(cursor, [&](CXCursor cur) {
            auto loc = clang_getCursorLocation(cur);
            if (clang_Location_isFromMainFile(loc))
                return visitFnSignature(cur, loc, fnNode, ignoreSignature);
            return CXChildVisit_Continue;
        });
        return ret ? CXChildVisit_Break : CXChildVisit_Continue;
    }

    Node *nodeForCommentAtLocation(CXSourceLocation loc, CXSourceLocation nextCommentLoc);

private:
    /*!
      SimpleLoc represents a simple location in the main source file,
      which can be used as a key in a QMap.
     */
    struct SimpleLoc
    {
        unsigned int line {}, column {};
        friend bool operator<(const SimpleLoc &a, const SimpleLoc &b)
        {
            return a.line != b.line ? a.line < b.line : a.column < b.column;
        }
    };
    /*!
      \variable ClangVisitor::declMap_
      Map of all the declarations in the source file so we can match them
      with a documentation comment.
     */
    QMap<SimpleLoc, CXCursor> declMap_;

    QDocDatabase *qdb_;
    Aggregate *parent_;
    const QMultiHash<QString, QString> allHeaders_;
    QHash<CXFile, bool> isInterestingCache_; // doing a canonicalFilePath is slow, so keep a cache.

    /*!
        Returns true if the symbol should be ignored for the documentation.
     */
    bool ignoredSymbol(const QString &symbolName)
    {
        if (symbolName == QLatin1String("QPrivateSignal"))
            return true;
        // Ignore functions generated by property macros
        if (symbolName.startsWith("_qt_property_"))
            return true;
        // Ignore template argument deduction guides
        if (symbolName.startsWith("<deduction guide"))
            return true;
        return false;
    }

    /*!
        The type parameters do not need to be fully qualified
        This function removes the ClassName:: if needed.

        example: 'QLinkedList::iterator' -> 'iterator'
     */
    QString adjustTypeName(const QString &typeName)
    {
        auto parent = parent_->parent();
        if (parent && parent->isClassNode()) {
            QStringView typeNameConstRemoved(typeName);
            if (typeNameConstRemoved.startsWith(QLatin1String("const ")))
                typeNameConstRemoved = typeNameConstRemoved.mid(6);

            auto parentName = parent->fullName();
            if (typeNameConstRemoved.startsWith(parentName)
                && typeNameConstRemoved.mid(parentName.size(), 2) == QLatin1String("::")) {
                QString result = typeName;
                result.remove(typeName.indexOf(typeNameConstRemoved), parentName.size() + 2);
                return result;
            }
        }
        return typeName;
    }

    CXChildVisitResult visitSource(CXCursor cursor, CXSourceLocation loc);
    CXChildVisitResult visitHeader(CXCursor cursor, CXSourceLocation loc);
    CXChildVisitResult visitFnSignature(CXCursor cursor, CXSourceLocation loc, Node **fnNode,
                                        bool &ignoreSignature);
    void processFunction(FunctionNode *fn, CXCursor cursor);
    bool parseProperty(const QString &spelling, const Location &loc);
    void readParameterNamesAndAttributes(FunctionNode *fn, CXCursor cursor);
    Aggregate *getSemanticParent(CXCursor cursor);
};

/*!
  Visits a cursor in the .cpp file.
  This fills the declMap_
 */
CXChildVisitResult ClangVisitor::visitSource(CXCursor cursor, CXSourceLocation loc)
{
    auto kind = clang_getCursorKind(cursor);
    if (clang_isDeclaration(kind)) {
        SimpleLoc l;
        clang_getPresumedLocation(loc, nullptr, &l.line, &l.column);
        declMap_.insert(l, cursor);
        return CXChildVisit_Recurse;
    }
    return CXChildVisit_Continue;
}

/*!
  If the semantic and lexical parent cursors of \a cursor are
  not the same, find the Aggregate node for the semantic parent
  cursor and return it. Otherwise return the current parent.
 */
Aggregate *ClangVisitor::getSemanticParent(CXCursor cursor)
{
    CXCursor sp = clang_getCursorSemanticParent(cursor);
    CXCursor lp = clang_getCursorLexicalParent(cursor);
    if (!clang_equalCursors(sp, lp) && clang_isDeclaration(clang_getCursorKind(sp))) {
        Node *spn = findNodeForCursor(qdb_, sp);
        if (spn && spn->isAggregate()) {
            return static_cast<Aggregate *>(spn);
        }
    }
    return parent_;
}

CXChildVisitResult ClangVisitor::visitFnSignature(CXCursor cursor, CXSourceLocation, Node **fnNode,
                                                  bool &ignoreSignature)
{
    switch (clang_getCursorKind(cursor)) {
    case CXCursor_Namespace:
        return CXChildVisit_Recurse;
    case CXCursor_FunctionDecl:
    case CXCursor_FunctionTemplate:
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_ConversionFunction: {
        ignoreSignature = false;
        if (ignoredSymbol(functionName(cursor))) {
            *fnNode = nullptr;
            ignoreSignature = true;
        } else {
            *fnNode = findNodeForCursor(qdb_, cursor);
            if (*fnNode) {
                if ((*fnNode)->isFunction(Node::CPP)) {
                    auto *fn = static_cast<FunctionNode *>(*fnNode);
                    readParameterNamesAndAttributes(fn, cursor);
                }
            } else { // Possibly an implicitly generated special member
                QString name = functionName(cursor);
                if (ignoredSymbol(name))
                    return CXChildVisit_Continue;
                Aggregate *semanticParent = getSemanticParent(cursor);
                if (semanticParent && semanticParent->isClass()) {
                    auto *candidate = new FunctionNode(nullptr, name);
                    processFunction(candidate, cursor);
                    if (!candidate->isSpecialMemberFunction()) {
                        delete candidate;
                        return CXChildVisit_Continue;
                    }
                    candidate->setDefault(true);
                    semanticParent->addChild(*fnNode = candidate);
                }
            }
        }
        break;
    }
    default:
        break;
    }
    return CXChildVisit_Continue;
}

CXChildVisitResult ClangVisitor::visitHeader(CXCursor cursor, CXSourceLocation loc)
{
    auto kind = clang_getCursorKind(cursor);
    QString templateString;
    switch (kind) {
    case CXCursor_TypeAliasTemplateDecl:
    case CXCursor_TypeAliasDecl: {
        QString aliasDecl = getSpelling(clang_getCursorExtent(cursor)).simplified();
        QStringList typeAlias = aliasDecl.split(QLatin1Char('='));
        if (typeAlias.size() == 2) {
            typeAlias[0] = typeAlias[0].trimmed();
            const QLatin1String usingString("using ");
            qsizetype usingPos = typeAlias[0].indexOf(usingString);
            if (usingPos != -1) {
                if (kind == CXCursor_TypeAliasTemplateDecl)
                    templateString = typeAlias[0].left(usingPos).trimmed();
                typeAlias[0].remove(0, usingPos + usingString.size());
                typeAlias[0] = typeAlias[0].split(QLatin1Char(' ')).first();
                typeAlias[1] = typeAlias[1].trimmed();
                auto *ta = new TypeAliasNode(parent_, typeAlias[0], typeAlias[1]);
                ta->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
                ta->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
                ta->setTemplateDecl(templateString);
            }
        }
        return CXChildVisit_Continue;
    }
    case CXCursor_StructDecl:
    case CXCursor_UnionDecl:
        if (fromCXString(clang_getCursorSpelling(cursor)).isEmpty()) // anonymous struct or union
            return CXChildVisit_Continue;
        Q_FALLTHROUGH();
    case CXCursor_ClassTemplate:
        templateString = templateDecl(cursor);
        Q_FALLTHROUGH();
    case CXCursor_ClassDecl: {
        if (!clang_isCursorDefinition(cursor))
            return CXChildVisit_Continue;

        if (findNodeForCursor(qdb_, cursor)) // Was already parsed, probably in another TU
            return CXChildVisit_Continue;

        QString className = fromCXString(clang_getCursorSpelling(cursor));

        Aggregate *semanticParent = getSemanticParent(cursor);
        if (semanticParent && semanticParent->findNonfunctionChild(className, &Node::isClassNode)) {
            return CXChildVisit_Continue;
        }

        CXCursorKind actualKind = (kind == CXCursor_ClassTemplate) ?
                 clang_getTemplateCursorKind(cursor) : kind;

        Node::NodeType type = Node::Class;
        if (actualKind == CXCursor_StructDecl)
            type = Node::Struct;
        else if (actualKind == CXCursor_UnionDecl)
            type = Node::Union;

        auto *classe = new ClassNode(type, semanticParent, className);
        classe->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
        classe->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));

        if (kind == CXCursor_ClassTemplate)
            classe->setTemplateDecl(templateString);

        QScopedValueRollback<Aggregate *> setParent(parent_, classe);
        return visitChildren(cursor);
    }
    case CXCursor_CXXBaseSpecifier: {
        if (!parent_->isClassNode())
            return CXChildVisit_Continue;
        auto access = fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor));
        auto type = clang_getCursorType(cursor);
        auto baseCursor = clang_getTypeDeclaration(type);
        auto baseNode = findNodeForCursor(qdb_, baseCursor);
        auto classe = static_cast<ClassNode *>(parent_);
        if (baseNode == nullptr || !baseNode->isClassNode()) {
            QString bcName = reconstructQualifiedPathForCursor(baseCursor);
            classe->addUnresolvedBaseClass(access,
                                           bcName.split(QLatin1String("::"), Qt::SkipEmptyParts));
            return CXChildVisit_Continue;
        }
        auto baseClasse = static_cast<ClassNode *>(baseNode);
        classe->addResolvedBaseClass(access, baseClasse);
        return CXChildVisit_Continue;
    }
    case CXCursor_Namespace: {
        QString namespaceName = fromCXString(clang_getCursorDisplayName(cursor));
        NamespaceNode *ns = nullptr;
        if (parent_)
            ns = static_cast<NamespaceNode *>(
                    parent_->findNonfunctionChild(namespaceName, &Node::isNamespace));
        if (!ns) {
            ns = new NamespaceNode(parent_, namespaceName);
            ns->setAccess(Access::Public);
            ns->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
        }
        QScopedValueRollback<Aggregate *> setParent(parent_, ns);
        return visitChildren(cursor);
    }
    case CXCursor_FunctionTemplate:
        templateString = templateDecl(cursor);
        Q_FALLTHROUGH();
    case CXCursor_FunctionDecl:
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_ConversionFunction: {
        if (findNodeForCursor(qdb_, cursor)) // Was already parsed, probably in another TU
            return CXChildVisit_Continue;
        QString name = functionName(cursor);
        if (ignoredSymbol(name))
            return CXChildVisit_Continue;
        // constexpr constructors generate also a global instance; ignore
        if (kind == CXCursor_Constructor && parent_ == qdb_->primaryTreeRoot())
            return CXChildVisit_Continue;

        auto *fn = new FunctionNode(parent_, name);
        CXSourceRange range = clang_Cursor_getCommentRange(cursor);
        if (!clang_Range_isNull(range)) {
            QString comment = getSpelling(range);
            if (comment.startsWith("//!")) {
                qsizetype tag = comment.indexOf(QChar('['));
                if (tag > 0) {
                    qsizetype end = comment.indexOf(QChar(']'), ++tag);
                    if (end > 0)
                        fn->setTag(comment.mid(tag, end - tag));
                }
            }
        }
        processFunction(fn, cursor);
        fn->setTemplateDecl(templateString);
        return CXChildVisit_Continue;
    }
#if CINDEX_VERSION >= 36
    case CXCursor_FriendDecl: {
        return visitChildren(cursor);
    }
#endif
    case CXCursor_EnumDecl: {
        auto *en = static_cast<EnumNode *>(findNodeForCursor(qdb_, cursor));
        if (en && en->items().size())
            return CXChildVisit_Continue; // Was already parsed, probably in another TU
        QString enumTypeName = fromCXString(clang_getCursorSpelling(cursor));
        if (enumTypeName.isEmpty()) {
            enumTypeName = "anonymous";
            if (parent_ && (parent_->isClassNode() || parent_->isNamespace())) {
                Node *n = parent_->findNonfunctionChild(enumTypeName, &Node::isEnumType);
                if (n)
                    en = static_cast<EnumNode *>(n);
            }
        }
        if (!en) {
            en = new EnumNode(parent_, enumTypeName, clang_EnumDecl_isScoped(cursor));
            en->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
            en->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
        }

        // Enum values
        visitChildrenLambda(cursor, [&](CXCursor cur) {
            if (clang_getCursorKind(cur) != CXCursor_EnumConstantDecl)
                return CXChildVisit_Continue;

            QString value;
            visitChildrenLambda(cur, [&](CXCursor cur) {
                if (clang_isExpression(clang_getCursorKind(cur))) {
                    value = getSpelling(clang_getCursorExtent(cur));
                    return CXChildVisit_Break;
                }
                return CXChildVisit_Continue;
            });
            if (value.isEmpty()) {
                QLatin1String hex("0x");
                if (!en->items().isEmpty() && en->items().last().value().startsWith(hex)) {
                    value = hex + QString::number(clang_getEnumConstantDeclValue(cur), 16);
                } else {
                    value = QString::number(clang_getEnumConstantDeclValue(cur));
                }
            }

            en->addItem(EnumItem(fromCXString(clang_getCursorSpelling(cur)), value));
            return CXChildVisit_Continue;
        });
        return CXChildVisit_Continue;
    }
    case CXCursor_FieldDecl:
    case CXCursor_VarDecl: {
        if (findNodeForCursor(qdb_, cursor)) // Was already parsed, probably in another TU
            return CXChildVisit_Continue;

        auto access = fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor));
        auto var = new VariableNode(parent_, fromCXString(clang_getCursorSpelling(cursor)));
        var->setAccess(access);
        var->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
        var->setLeftType(fromCXString(clang_getTypeSpelling(clang_getCursorType(cursor))));
        var->setStatic(kind == CXCursor_VarDecl && parent_->isClassNode());
        return CXChildVisit_Continue;
    }
    case CXCursor_TypedefDecl: {
        if (findNodeForCursor(qdb_, cursor)) // Was already parsed, probably in another TU
            return CXChildVisit_Continue;
        auto *td = new TypedefNode(parent_, fromCXString(clang_getCursorSpelling(cursor)));
        td->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
        td->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
        // Search to see if this is a Q_DECLARE_FLAGS  (if the type is QFlags<ENUM>)
        visitChildrenLambda(cursor, [&](CXCursor cur) {
            if (clang_getCursorKind(cur) != CXCursor_TemplateRef
                || fromCXString(clang_getCursorSpelling(cur)) != QLatin1String("QFlags"))
                return CXChildVisit_Continue;
            // Found QFlags<XXX>
            visitChildrenLambda(cursor, [&](CXCursor cur) {
                if (clang_getCursorKind(cur) != CXCursor_TypeRef)
                    return CXChildVisit_Continue;
                auto *en =
                        findNodeForCursor(qdb_, clang_getTypeDeclaration(clang_getCursorType(cur)));
                if (en && en->isEnumType())
                    static_cast<EnumNode *>(en)->setFlagsType(td);
                return CXChildVisit_Break;
            });
            return CXChildVisit_Break;
        });
        return CXChildVisit_Continue;
    }
    default:
        if (clang_isDeclaration(kind) && parent_->isClassNode()) {
            // may be a property macro or a static_assert
            // which is not exposed from the clang API
            parseProperty(getSpelling(clang_getCursorExtent(cursor)),
                          fromCXSourceLocation(loc));
        }
        return CXChildVisit_Continue;
    }
}

void ClangVisitor::readParameterNamesAndAttributes(FunctionNode *fn, CXCursor cursor)
{
    Parameters &parameters = fn->parameters();
    // Visit the parameters and attributes
    int i = 0;
    visitChildrenLambda(cursor, [&](CXCursor cur) {
        auto kind = clang_getCursorKind(cur);
        if (kind == CXCursor_AnnotateAttr) {
            QString annotation = fromCXString(clang_getCursorDisplayName(cur));
            if (annotation == QLatin1String("qt_slot")) {
                fn->setMetaness(FunctionNode::Slot);
            } else if (annotation == QLatin1String("qt_signal")) {
                fn->setMetaness(FunctionNode::Signal);
            }
            if (annotation == QLatin1String("qt_invokable"))
                fn->setInvokable(true);
        } else if (kind == CXCursor_CXXOverrideAttr) {
            fn->setOverride(true);
        } else if (kind == CXCursor_ParmDecl) {
            if (i >= parameters.count())
                return CXChildVisit_Break; // Attributes comes before parameters so we can break.
            QString name = fromCXString(clang_getCursorSpelling(cur));
            if (!name.isEmpty()) {
                parameters[i].setName(name);
                // Find the default value
                visitChildrenLambda(cur, [&](CXCursor cur) {
                    if (clang_isExpression(clang_getCursorKind(cur))) {
                        QString defaultValue = getSpelling(clang_getCursorExtent(cur));
                        if (defaultValue.startsWith('=')) // In some cases, the = is part of the range.
                            defaultValue = QStringView{defaultValue}.mid(1).trimmed().toString();
                        if (defaultValue.isEmpty())
                            defaultValue = QStringLiteral("...");
                        parameters[i].setDefaultValue(defaultValue);
                        return CXChildVisit_Break;
                    }
                    return CXChildVisit_Continue;
                });
            }
            ++i;
        }
        return CXChildVisit_Continue;
    });
}

/*!
 * Returns the underlying Decl that \a cursor represents.
 *
 * This can be used to drop back down from a LibClang's CXCursor to
 * the underlying C++ AST that Clang provides.
 *
 * It should be used when LibClang does not expose certain
 * functionalities that are available in the C++ AST.
 *
 * The CXCursor should represent a declaration. Usages of this
 * function on CXCursors that do not represent a declaration may
 * produce undefined results.
 */
static const clang::Decl* get_cursor_declaration(CXCursor cursor) {
    assert(clang_isDeclaration(clang_getCursorKind(cursor)));

    return static_cast<const clang::Decl*>(cursor.data[0]);
}

void ClangVisitor::processFunction(FunctionNode *fn, CXCursor cursor)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    CXType funcType = clang_getCursorType(cursor);
    fn->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
    fn->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
    if (kind == CXCursor_Constructor
        // a constructor template is classified as CXCursor_FunctionTemplate
        || (kind == CXCursor_FunctionTemplate && fn->name() == parent_->name()))
        fn->setMetaness(FunctionNode::Ctor);
    else if (kind == CXCursor_Destructor)
        fn->setMetaness(FunctionNode::Dtor);
    else
        fn->setReturnType(adjustTypeName(
                fromCXString(clang_getTypeSpelling(clang_getResultType(funcType)))));

    fn->setStatic(clang_CXXMethod_isStatic(cursor));
    fn->setConst(clang_CXXMethod_isConst(cursor));
    fn->setVirtualness(!clang_CXXMethod_isVirtual(cursor)
                               ? FunctionNode::NonVirtual
                               : clang_CXXMethod_isPureVirtual(cursor)
                                       ? FunctionNode::PureVirtual
                                       : FunctionNode::NormalVirtual);

    // REMARK: We assume that the following operations and casts are
    // generally safe.
    // Callers of those methods will generally check at the LibClang
    // level the kind of cursor we are dealing with and will pass on
    // only valid cursors that are of a function kind and that are at
    // least a declaration.
    //
    // Failure to do so implies a bug in the call chain and should be
    // dealt with as such.
    const clang::Decl* declaration = get_cursor_declaration(cursor);
    const clang::FunctionDecl* function_declaration{nullptr};
    if (auto templated_decl = llvm::dyn_cast_or_null<const clang::FunctionTemplateDecl>(declaration))
        function_declaration = templated_decl->getTemplatedDecl();
    else function_declaration = static_cast<const clang::FunctionDecl*>(declaration);

    assert(function_declaration);

    const clang::CXXConstructorDecl* constructor_declaration = llvm::dyn_cast<const clang::CXXConstructorDecl>(function_declaration);

    if (constructor_declaration && constructor_declaration->isCopyConstructor()) fn->setMetaness(FunctionNode::CCtor);
    else if (constructor_declaration && constructor_declaration->isMoveConstructor()) fn->setMetaness(FunctionNode::MCtor);

    const clang::CXXConversionDecl* conversion_declaration = llvm::dyn_cast<const clang::CXXConversionDecl>(function_declaration);

    if (function_declaration->isConstexpr()) fn->markConstexpr();
    if (
        (constructor_declaration && constructor_declaration->isExplicit()) ||
        (conversion_declaration && conversion_declaration->isExplicit())
    ) fn->markExplicit();

    const clang::CXXMethodDecl* method_declaration = llvm::dyn_cast<const clang::CXXMethodDecl>(function_declaration);

    if (method_declaration && method_declaration->isCopyAssignmentOperator()) fn->setMetaness(FunctionNode::CAssign);
    else if (method_declaration && method_declaration->isMoveAssignmentOperator()) fn->setMetaness(FunctionNode::MAssign);

    const clang::FunctionType* function_type = function_declaration->getFunctionType();
    const clang::FunctionProtoType* function_prototype = static_cast<const clang::FunctionProtoType*>(function_type);

    if (function_prototype) {
        clang::FunctionProtoType::ExceptionSpecInfo exception_specification = function_prototype->getExceptionSpecInfo();

        if (exception_specification.Type != clang::ExceptionSpecificationType::EST_None) {
            clang::SourceManager& source_manager = function_declaration->getASTContext().getSourceManager();
            const clang::LangOptions& lang_options = function_declaration->getASTContext().getLangOpts();

            fn->markNoexcept(
                exception_specification.NoexceptExpr ?
                    QString::fromStdString(clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(exception_specification.NoexceptExpr->getSourceRange()), source_manager, lang_options).str()) :
                    ""
            );
        }
    }

    CXRefQualifierKind refQualKind = clang_Type_getCXXRefQualifier(funcType);
    if (refQualKind == CXRefQualifier_LValue)
        fn->setRef(true);
    else if (refQualKind == CXRefQualifier_RValue)
        fn->setRefRef(true);
    // For virtual functions, determine what it overrides
    // (except for destructor for which we do not want to classify as overridden)
    if (!fn->isNonvirtual() && kind != CXCursor_Destructor)
        setOverridesForFunction(fn, cursor);

    Parameters &parameters = fn->parameters();
    parameters.clear();
    parameters.reserve(function_declaration->getNumParams());

    const clang::LangOptions& lang_options = function_declaration->getASTContext().getLangOpts();
    clang::PrintingPolicy p{lang_options};

    for (clang::ParmVarDecl* const parameter_declaration : function_declaration->parameters()) {
        clang::QualType parameter_type = parameter_declaration->getOriginalType();

        parameters.append(adjustTypeName(QString::fromStdString(parameter_type.getAsString(p))));

        if (!parameter_type.isCanonical())
            parameters.last().setCanonicalType(QString::fromStdString(parameter_type.getCanonicalType().getAsString(p)));
    }


    if (parameters.count() > 0) {
        if (parameters.last().type().endsWith(QLatin1String("QPrivateSignal"))) {
            parameters.pop_back(); // remove the QPrivateSignal argument
            parameters.setPrivateSignal();
        }
    }
    if (clang_isFunctionTypeVariadic(funcType))
        parameters.append(QStringLiteral("..."));
    readParameterNamesAndAttributes(fn, cursor);

    if (declaration->getFriendObjectKind() != clang::Decl::FOK_None)
        fn->setRelatedNonmember(true);
}

bool ClangVisitor::parseProperty(const QString &spelling, const Location &loc)
{
    if (!spelling.startsWith(QLatin1String("Q_PROPERTY"))
        && !spelling.startsWith(QLatin1String("QDOC_PROPERTY"))
        && !spelling.startsWith(QLatin1String("Q_OVERRIDE")))
        return false;

    qsizetype lpIdx = spelling.indexOf(QChar('('));
    qsizetype rpIdx = spelling.lastIndexOf(QChar(')'));
    if (lpIdx <= 0 || rpIdx <= lpIdx)
        return false;

    QString signature = spelling.mid(lpIdx + 1, rpIdx - lpIdx - 1);
    signature = signature.simplified();
    QStringList parts = signature.split(QChar(' '), Qt::SkipEmptyParts);

    static const QStringList attrs =
            QStringList() << "READ"     << "MEMBER"     << "WRITE"
                          << "NOTIFY"   << "CONSTANT"   << "FINAL"
                          << "REQUIRED" << "BINDABLE"   << "DESIGNABLE"
                          << "RESET"    << "REVISION"   << "SCRIPTABLE"
                          << "STORED"   << "USER";

    // Find the location of the first attribute. All preceding parts
    // represent the property type + name.
    auto it = std::find_if(parts.cbegin(), parts.cend(),
                           [](const QString &attr) -> bool {
        return attrs.contains(attr);
    });

    if (it == parts.cend() || std::distance(parts.cbegin(), it) < 2)
        return false;

    QStringList typeParts;
    std::copy(parts.cbegin(), it, std::back_inserter(typeParts));
    parts.erase(parts.cbegin(), it);
    QString name = typeParts.takeLast();

    // Move the pointer operator(s) from name to type
    while (!name.isEmpty() && name.front() == QChar('*')) {
        typeParts.last().push_back(name.front());
        name.removeFirst();
    }

    // Need at least READ or MEMBER + getter/member name
    if (parts.size() < 2 || name.isEmpty())
        return false;

    auto *property = new PropertyNode(parent_, name);
    property->setAccess(Access::Public);
    property->setLocation(loc);
    property->setDataType(typeParts.join(QChar(' ')));

    int i = 0;
    while (i < parts.size()) {
        const QString &key = parts.at(i++);
        // Keywords with no associated values
        if (key == "CONSTANT") {
            property->setConstant();
        } else if (key == "REQUIRED") {
            property->setRequired();
        }
        if (i < parts.size()) {
            QString value = parts.at(i++);
            if (key == "READ") {
                qdb_->addPropertyFunction(property, value, PropertyNode::FunctionRole::Getter);
            } else if (key == "WRITE") {
                qdb_->addPropertyFunction(property, value, PropertyNode::FunctionRole::Setter);
                property->setWritable(true);
            } else if (key == "MEMBER") {
                property->setWritable(true);
            } else if (key == "STORED") {
                property->setStored(value.toLower() == "true");
            } else if (key == "BINDABLE") {
                property->setPropertyType(PropertyNode::PropertyType::BindableProperty);
                qdb_->addPropertyFunction(property, value, PropertyNode::FunctionRole::Bindable);
            } else if (key == "RESET") {
                qdb_->addPropertyFunction(property, value, PropertyNode::FunctionRole::Resetter);
            } else if (key == "NOTIFY") {
                qdb_->addPropertyFunction(property, value, PropertyNode::FunctionRole::Notifier);
            }
        }
    }
    return true;
}

/*!
  Given a comment at location \a loc, return a Node for this comment
  \a nextCommentLoc is the location of the next comment so the declaration
  must be inbetween.
  Returns nullptr if no suitable declaration was found between the two comments.
 */
Node *ClangVisitor::nodeForCommentAtLocation(CXSourceLocation loc, CXSourceLocation nextCommentLoc)
{
    ClangVisitor::SimpleLoc docloc;
    clang_getPresumedLocation(loc, nullptr, &docloc.line, &docloc.column);
    auto decl_it = declMap_.upperBound(docloc);
    if (decl_it == declMap_.end())
        return nullptr;

    unsigned int declLine = decl_it.key().line;
    unsigned int nextCommentLine;
    clang_getPresumedLocation(nextCommentLoc, nullptr, &nextCommentLine, nullptr);
    if (nextCommentLine < declLine)
        return nullptr; // there is another comment before the declaration, ignore it.

    // make sure the previous decl was finished.
    if (decl_it != declMap_.begin()) {
        CXSourceLocation prevDeclEnd = clang_getRangeEnd(clang_getCursorExtent(*(std::prev(decl_it))));
        unsigned int prevDeclLine;
        clang_getPresumedLocation(prevDeclEnd, nullptr, &prevDeclLine, nullptr);
        if (prevDeclLine >= docloc.line) {
            // The previous declaration was still going. This is only valid if the previous
            // declaration is a parent of the next declaration.
            auto parent = clang_getCursorLexicalParent(*decl_it);
            if (!clang_equalCursors(parent, *(std::prev(decl_it))))
                return nullptr;
        }
    }
    auto *node = findNodeForCursor(qdb_, *decl_it);
    // borrow the parameter name from the definition
    if (node && node->isFunction(Node::CPP))
        readParameterNamesAndAttributes(static_cast<FunctionNode *>(node), *decl_it);
    return node;
}

/*!
  Get the include paths from the qdoc configuration database
  \a config. Call the initializeParser() in the base class.
  Get the defines list from the qdocconf database.

  \note on \macos and Linux, we try to also query the system
  and framework (\macos) include paths from the compiler.
 */
void ClangCodeParser::initializeParser()
{
    Config &config = Config::instance();
    auto args = config.getCanonicalPathList(CONFIG_INCLUDEPATHS,
                                            Config::IncludePaths);
#ifdef Q_OS_MACOS
    args.append(Utilities::getInternalIncludePaths(QStringLiteral("clang++")));
#elif defined(Q_OS_LINUX)
    args.append(Utilities::getInternalIncludePaths(QStringLiteral("g++")));
#endif
    m_includePaths.clear();
    for (const auto &path : std::as_const(args)) {
        if (!path.isEmpty())
            m_includePaths.append(path.toUtf8());
    }
    m_includePaths.erase(std::unique(m_includePaths.begin(), m_includePaths.end()),
                         m_includePaths.end());
    m_pchFileDir.reset(nullptr);
    m_allHeaders.clear();
    m_pchName.clear();
    m_defines.clear();
    QSet<QString> accepted;
    {
        const QStringList tmpDefines{config.get(CONFIG_CLANGDEFINES).asStringList()};
        for (const QString &def : tmpDefines) {
            if (!accepted.contains(def)) {
                QByteArray tmp("-D");
                tmp.append(def.toUtf8());
                m_defines.append(tmp.constData());
                accepted.insert(def);
            }
        }
    }
    {
        const QStringList tmpDefines{config.get(CONFIG_DEFINES).asStringList()};
        for (const QString &def : tmpDefines) {
            if (!accepted.contains(def) && !def.contains(QChar('*'))) {
                QByteArray tmp("-D");
                tmp.append(def.toUtf8());
                m_defines.append(tmp.constData());
                accepted.insert(def);
            }
        }
    }
    qCDebug(lcQdoc).nospace() << __FUNCTION__ << " Clang v" << CINDEX_VERSION_MAJOR << '.'
                              << CINDEX_VERSION_MINOR;
}

/*!
 */
QString ClangCodeParser::language()
{
    return "Clang";
}

/*!
  Returns a list of extensions for source files, i.e. not
  header files.
 */
QStringList ClangCodeParser::sourceFileNameFilter()
{
    return QStringList() << "*.c++"
                         << "*.cc"
                         << "*.cpp"
                         << "*.cxx"
                         << "*.mm";
}

/*!
  Parse the C++ header file identified by \a filePath and add
  the parsed contents to the database. The \a location is used
  for reporting errors.
 */
void ClangCodeParser::parseHeaderFile(const Location & /*location*/, const QString &filePath)
{
    QFileInfo fi(filePath);
    const QString &fileName = fi.fileName();
    const QString &canonicalPath = fi.canonicalPath();

    if (m_allHeaders.contains(fileName, canonicalPath))
        return;

    m_allHeaders.insert(fileName, canonicalPath);
}

static const char *defaultArgs_[] = {
/*
  https://bugreports.qt.io/browse/QTBUG-94365
  An unidentified bug in Clang 15.x causes parsing failures due to errors in
  the AST. This replicates only with C++20 support enabled - avoid the issue
  by using C++17 with Clang 15.
 */
#if LIBCLANG_VERSION_MAJOR == 15
    "-std=c++17",
#else
    "-std=c++20",
#endif
#ifndef Q_OS_WIN
    "-fPIC",
#else
    "-fms-compatibility-version=19",
#endif
    "-DQ_QDOC",
    "-DQ_CLANG_QDOC",
    "-DQT_DISABLE_DEPRECATED_UP_TO=0",
    "-DQT_ANNOTATE_CLASS(type,...)=static_assert(sizeof(#__VA_ARGS__),#type);",
    "-DQT_ANNOTATE_CLASS2(type,a1,a2)=static_assert(sizeof(#a1,#a2),#type);",
    "-DQT_ANNOTATE_FUNCTION(a)=__attribute__((annotate(#a)))",
    "-DQT_ANNOTATE_ACCESS_SPECIFIER(a)=__attribute__((annotate(#a)))",
    "-Wno-constant-logical-operand",
    "-Wno-macro-redefined",
    "-Wno-nullability-completeness",
    "-fvisibility=default",
    "-ferror-limit=0",
    ("-I" CLANG_RESOURCE_DIR)
};

/*!
  Load the default arguments and the defines into \a args.
  Clear \a args first.
 */
void ClangCodeParser::getDefaultArgs()
{
    m_args.clear();
    m_args.insert(m_args.begin(), std::begin(defaultArgs_), std::end(defaultArgs_));
    // Add the defines from the qdocconf file.
    for (const auto &p : std::as_const(m_defines))
        m_args.push_back(p.constData());
}

static QList<QByteArray> includePathsFromHeaders(const QMultiHash<QString, QString> &allHeaders)
{
    QList<QByteArray> result;
    for (auto it = allHeaders.cbegin(); it != allHeaders.cend(); ++it) {
        const QByteArray path = "-I" + it.value().toLatin1();
        const QByteArray parent =
                "-I" + QDir::cleanPath(it.value() + QLatin1String("/../")).toLatin1();
        if (!result.contains(path))
            result.append(path);
        if (!result.contains(parent))
            result.append(parent);
    }
    return result;
}

/*!
  Load the include paths into \a moreArgs. If no include paths
  were provided, try to guess reasonable include paths.
 */
void ClangCodeParser::getMoreArgs()
{
    if (m_includePaths.isEmpty()) {
        /*
          The include paths provided are inadequate. Make a list
          of reasonable places to look for include files and use
          that list instead.
         */
        qCWarning(lcQdoc) << "No include paths passed to qdoc; guessing reasonable include paths";

        QString basicIncludeDir = QDir::cleanPath(QString(Config::installDir + "/../include"));
        m_moreArgs += "-I" + basicIncludeDir.toLatin1();
        m_moreArgs += includePathsFromHeaders(m_allHeaders);
    } else {
        m_moreArgs = m_includePaths;
    }
}

/*!
  Building the PCH must be possible when there are no .cpp
  files, so it is moved here to its own member function, and
  it is called after the list of header files is complete.
 */
void ClangCodeParser::buildPCH(QString module_header)
{
    if (m_pchFileDir) return;
    if (module_header.isEmpty()) return;

    m_pchFileDir.reset(new QTemporaryDir(QDir::tempPath() + QLatin1String("/qdoc_pch")));
    if (m_pchFileDir->isValid()) {
        const QByteArray module = module_header.toUtf8();
        QByteArray header;

        qCDebug(lcQdoc) << "Build and visit PCH for" << module_header;
        // A predicate for std::find_if() to locate a path to the module's header
        // (e.g. QtGui/QtGui) to be used as pre-compiled header
        struct FindPredicate
        {
            enum SearchType { Any, Module };
            QByteArray &candidate_;
            const QByteArray &module_;
            SearchType type_;
            FindPredicate(QByteArray &candidate, const QByteArray &module,
                            SearchType type = Any)
                : candidate_(candidate), module_(module), type_(type)
            {
            }

            bool operator()(const QByteArray &p) const
            {
                if (type_ != Any && !p.endsWith(module_))
                    return false;
                candidate_ = p + "/";
                candidate_.append(module_);
                if (p.startsWith("-I"))
                    candidate_ = candidate_.mid(2);
                return QFile::exists(QString::fromUtf8(candidate_));
            }
        };

        // First, search for an include path that contains the module name, then any path
        QByteArray candidate;
        auto it = std::find_if(m_includePaths.begin(), m_includePaths.end(),
                                FindPredicate(candidate, module, FindPredicate::Module));
        if (it == m_includePaths.end())
            it = std::find_if(m_includePaths.begin(), m_includePaths.end(),
                                FindPredicate(candidate, module, FindPredicate::Any));
        if (it != m_includePaths.end())
            header = candidate;

        if (header.isEmpty()) {
            qWarning() << "(qdoc) Could not find the module header in include paths for module"
                        << module << "  (include paths: " << m_includePaths << ")";
            qWarning() << "       Artificial module header built from header dirs in qdocconf "
                            "file";
        }
        m_args.push_back("-xc++");
        CXTranslationUnit tu;
        QString tmpHeader = m_pchFileDir->path() + "/" + module;
        if (QFile tmpHeaderFile(tmpHeader); tmpHeaderFile.open(QIODevice::Text | QIODevice::WriteOnly)) {
            QTextStream out(&tmpHeaderFile);
            if (header.isEmpty()) {
                for (auto it = m_allHeaders.constKeyValueBegin();
                        it != m_allHeaders.constKeyValueEnd(); ++it) {
                    if (!(*it).first.endsWith(QLatin1String("_p.h"))
                        && !(*it).first.startsWith(QLatin1String("moc_"))) {
                        QString line = QLatin1String("#include \"") + (*it).second
                                + QLatin1String("/") + (*it).first + QLatin1String("\"");
                        out << line << "\n";
                    }
                }
            } else {
                QFileInfo headerFile(header);
                if (!headerFile.exists()) {
                    qWarning() << "Could not find module header file" << header;
                    return;
                }
                out << QLatin1String("#include \"") + header + QLatin1String("\"");
            }
        }

        CXErrorCode err =
                clang_parseTranslationUnit2(index_, tmpHeader.toLatin1().data(), m_args.data(),
                                            static_cast<int>(m_args.size()), nullptr, 0,
                                            flags_ | CXTranslationUnit_ForSerialization, &tu);
        qCDebug(lcQdoc) << __FUNCTION__ << "clang_parseTranslationUnit2(" << tmpHeader << m_args
                        << ") returns" << err;

        printDiagnostics(tu);

        if (!err && tu) {
            m_pchName = m_pchFileDir->path().toUtf8() + "/" + module + ".pch";
            auto error = clang_saveTranslationUnit(tu, m_pchName.constData(),
                                                    clang_defaultSaveOptions(tu));
            if (error) {
                qCCritical(lcQdoc) << "Could not save PCH file for" << module_header;
                m_pchName.clear();
            } else {
                // Visit the header now, as token from pre-compiled header won't be visited
                // later
                CXCursor cur = clang_getTranslationUnitCursor(tu);
                ClangVisitor visitor(m_qdb, m_allHeaders);
                visitor.visitChildren(cur);
                qCDebug(lcQdoc) << "PCH built and visited for" << module_header;
            }
        } else {
            m_pchFileDir->remove();
            qCCritical(lcQdoc) << "Could not create PCH file for " << module_header;
        }
        clang_disposeTranslationUnit(tu);
        m_args.pop_back(); // remove the "-xc++";
    }
}

/*!
  Precompile the header files for the current module.
 */
void ClangCodeParser::precompileHeaders(QString module_header)
{
    getDefaultArgs();
    getMoreArgs();
    for (const auto &p : std::as_const(m_moreArgs))
        m_args.push_back(p.constData());

    flags_ = static_cast<CXTranslationUnit_Flags>(CXTranslationUnit_Incomplete
                                                  | CXTranslationUnit_SkipFunctionBodies
                                                  | CXTranslationUnit_KeepGoing);

    index_ = clang_createIndex(1, kClangDontDisplayDiagnostics);

    buildPCH(module_header);
    clang_disposeIndex(index_);
}

static float getUnpatchedVersion(QString t)
{
    if (t.count(QChar('.')) > 1)
        t.truncate(t.lastIndexOf(QChar('.')));
    return t.toFloat();
}

/*!
  Get ready to parse the C++ cpp file identified by \a filePath
  and add its parsed contents to the database. \a location is
  used for reporting errors.

  Call matchDocsAndStuff() to do all the parsing and tree building.
 */
void ClangCodeParser::parseSourceFile(const Location & /*location*/, const QString &filePath, CppCodeParser& cpp_code_parser)
{
    /*
      The set of open namespaces is cleared before parsing
      each source file. The word "source" here means cpp file.
     */
    m_qdb->clearOpenNamespaces();
    flags_ = static_cast<CXTranslationUnit_Flags>(CXTranslationUnit_Incomplete
                                                  | CXTranslationUnit_SkipFunctionBodies
                                                  | CXTranslationUnit_KeepGoing);

    index_ = clang_createIndex(1, kClangDontDisplayDiagnostics);

    getDefaultArgs();
    if (!m_pchName.isEmpty() && !filePath.endsWith(".mm")) {
        m_args.push_back("-w");
        m_args.push_back("-include-pch");
        m_args.push_back(m_pchName.constData());
    }
    getMoreArgs();
    for (const auto &p : std::as_const(m_moreArgs))
        m_args.push_back(p.constData());

    CXTranslationUnit tu;
    CXErrorCode err =
            clang_parseTranslationUnit2(index_, filePath.toLocal8Bit(), m_args.data(),
                                        static_cast<int>(m_args.size()), nullptr, 0, flags_, &tu);
    qCDebug(lcQdoc) << __FUNCTION__ << "clang_parseTranslationUnit2(" << filePath << m_args
                    << ") returns" << err;
    printDiagnostics(tu);

    if (err || !tu) {
        qWarning() << "(qdoc) Could not parse source file" << filePath << " error code:" << err;
        clang_disposeTranslationUnit(tu);
        clang_disposeIndex(index_);
        return;
    }

    CXCursor tuCur = clang_getTranslationUnitCursor(tu);
    ClangVisitor visitor(m_qdb, m_allHeaders);
    visitor.visitChildren(tuCur);

    CXToken *tokens;
    unsigned int numTokens = 0;
    const QSet<QString> &commands = CppCodeParser::topic_commands + CppCodeParser::meta_commands;
    clang_tokenize(tu, clang_getCursorExtent(tuCur), &tokens, &numTokens);

    for (unsigned int i = 0; i < numTokens; ++i) {
        if (clang_getTokenKind(tokens[i]) != CXToken_Comment)
            continue;
        QString comment = fromCXString(clang_getTokenSpelling(tu, tokens[i]));
        if (!comment.startsWith("/*!"))
            continue;

        auto commentLoc = clang_getTokenLocation(tu, tokens[i]);
        auto loc = fromCXSourceLocation(commentLoc);
        auto end_loc = fromCXSourceLocation(clang_getRangeEnd(clang_getTokenExtent(tu, tokens[i])));
        Doc::trimCStyleComment(loc, comment);

        // Doc constructor parses the comment.
        Doc doc(loc, end_loc, comment, commands, CppCodeParser::topic_commands);
        if (cpp_code_parser.hasTooManyTopics(doc))
            continue;

        DocList docs;
        QString topic;
        NodeList nodes;
        const TopicList &topics = doc.topicsUsed();
        if (!topics.isEmpty())
            topic = topics[0].m_topic;

        if (topic.isEmpty()) {
            Node *n = nullptr;
            if (i + 1 < numTokens) {
                // Try to find the next declaration.
                CXSourceLocation nextCommentLoc = commentLoc;
                while (i + 2 < numTokens && clang_getTokenKind(tokens[i + 1]) != CXToken_Comment)
                    ++i; // already skip all the tokens that are not comments
                nextCommentLoc = clang_getTokenLocation(tu, tokens[i + 1]);
                n = visitor.nodeForCommentAtLocation(commentLoc, nextCommentLoc);
            }

            if (n) {
                nodes.append(n);
                docs.append(doc);
            } else if (CodeParser::isWorthWarningAbout(doc)) {
                bool future = false;
                if (doc.metaCommandsUsed().contains(COMMAND_SINCE)) {
                    QString sinceVersion = doc.metaCommandArgs(COMMAND_SINCE).at(0).first;
                    if (getUnpatchedVersion(sinceVersion) >
                        getUnpatchedVersion(Config::instance().get(CONFIG_VERSION).asString()))
                        future = true;
                }
                if (!future) {
                    doc.location().warning(
                            QStringLiteral("Cannot tie this documentation to anything"),
                            QStringLiteral("qdoc found a /*! ... */ comment, but there was no "
                                           "topic command (e.g., '\\%1', '\\%2') in the "
                                           "comment and no function definition following "
                                           "the comment.")
                                    .arg(COMMAND_FN, COMMAND_PAGE));
                }
            }
        } else {
            // Store the namespace scope from lexical parents of the comment
            m_namespaceScope.clear();
            CXCursor cur = clang_getCursor(tu, commentLoc);
            while (true) {
                CXCursorKind kind = clang_getCursorKind(cur);
                if (clang_isTranslationUnit(kind) || clang_isInvalid(kind))
                    break;
                if (kind == CXCursor_Namespace)
                    m_namespaceScope << fromCXString(clang_getCursorSpelling(cur));
                cur = clang_getCursorLexicalParent(cur);
            }
            cpp_code_parser.processTopicArgs(doc, topic, nodes, docs);
        }
        cpp_code_parser.processMetaCommands(nodes, docs);
    }

    clang_disposeTokens(tu, tokens, numTokens);
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index_);
    m_namespaceScope.clear();
    s_fn.clear();
}

/*!
  Use clang to parse the function signature from a function
  command. \a location is used for reporting errors. \a fnSignature
  is the string to parse. It is always a function decl.
 */
Node *ClangCodeParser::parseFnArg(const Location &location, const QString &fnSignature, const QString &idTag)
{
    Node *fnNode = nullptr;
    /*
      If the \fn command begins with a tag, then don't try to
      parse the \fn command with clang. Use the tag to search
      for the correct function node. It is an error if it can
      not be found. Return 0 in that case.
    */
    if (!idTag.isEmpty()) {
        fnNode = m_qdb->findFunctionNodeForTag(idTag);
        if (!fnNode) {
            location.error(
                    QStringLiteral("tag \\fn [%1] not used in any include file in current module").arg(idTag));
        } else {
            /*
              The function node was found. Use the formal
              parameter names from the \fn command, because
              they will be the names used in the documentation.
             */
            auto *fn = static_cast<FunctionNode *>(fnNode);
            QStringList leftParenSplit = fnSignature.mid(fnSignature.indexOf(fn->name())).split('(');
            if (leftParenSplit.size() > 1) {
                QStringList rightParenSplit = leftParenSplit[1].split(')');
                if (!rightParenSplit.empty()) {
                    QString params = rightParenSplit[0];
                    if (!params.isEmpty()) {
                        QStringList commaSplit = params.split(',');
                        Parameters &parameters = fn->parameters();
                        if (parameters.count() == commaSplit.size()) {
                            for (int i = 0; i < parameters.count(); ++i) {
                                QStringList blankSplit = commaSplit[i].split(' ', Qt::SkipEmptyParts);
                                if (blankSplit.size() > 1) {
                                    QString pName = blankSplit.last();
                                    // Remove any non-letters from the start of parameter name
                                    auto it = std::find_if(std::begin(pName), std::end(pName),
                                            [](const QChar &c) { return c.isLetter(); });
                                    parameters[i].setName(
                                            pName.remove(0, std::distance(std::begin(pName), it)));
                                }
                            }
                        }
                    }
                }
            }
        }
        return fnNode;
    }
    auto flags = static_cast<CXTranslationUnit_Flags>(CXTranslationUnit_Incomplete
                                                      | CXTranslationUnit_SkipFunctionBodies
                                                      | CXTranslationUnit_KeepGoing);

    CXIndex index = clang_createIndex(1, kClangDontDisplayDiagnostics);

    std::vector<const char *> args(std::begin(defaultArgs_), std::end(defaultArgs_));
    // Add the defines from the qdocconf file.
    for (const auto &p : std::as_const(m_defines))
        args.push_back(p.constData());
    if (!m_pchName.isEmpty()) {
        args.push_back("-w");
        args.push_back("-include-pch");
        args.push_back(m_pchName.constData());
    }
    CXTranslationUnit tu;
    s_fn.clear();
    for (const auto &ns : std::as_const(m_namespaceScope))
        s_fn.prepend("namespace " + ns.toUtf8() + " {");
    s_fn += fnSignature.toUtf8();
    if (!s_fn.endsWith(";"))
        s_fn += "{ }";
    s_fn.append(m_namespaceScope.size(), '}');

    const char *dummyFileName = fnDummyFileName;
    CXUnsavedFile unsavedFile { dummyFileName, s_fn.constData(),
                                static_cast<unsigned long>(s_fn.size()) };
    CXErrorCode err = clang_parseTranslationUnit2(index, dummyFileName, args.data(),
                                                  int(args.size()), &unsavedFile, 1, flags, &tu);
    qCDebug(lcQdoc) << __FUNCTION__ << "clang_parseTranslationUnit2(" << dummyFileName << args
                    << ") returns" << err;
    printDiagnostics(tu);
    if (err || !tu) {
        location.error(QStringLiteral("clang could not parse \\fn %1").arg(fnSignature));
        clang_disposeTranslationUnit(tu);
        clang_disposeIndex(index);
        return fnNode;
    } else {
        /*
          Always visit the tu if one is constructed, because
          it might be possible to find the correct node, even
          if clang detected diagnostics. Only bother to report
          the diagnostics if they stop us finding the node.
         */
        CXCursor cur = clang_getTranslationUnitCursor(tu);
        ClangVisitor visitor(m_qdb, m_allHeaders);
        bool ignoreSignature = false;
        visitor.visitFnArg(cur, &fnNode, ignoreSignature);
        /*
          If the visitor couldn't find a FunctionNode for the
          signature, then print the clang diagnostics if there
          were any.
         */
        if (fnNode == nullptr) {
            unsigned diagnosticCount = clang_getNumDiagnostics(tu);
            const auto &config = Config::instance();
            if (diagnosticCount > 0 && (!config.preparing() || config.singleExec())) {
                bool report = true;
                QStringList signature = fnSignature.split(QChar('('));
                if (signature.size() > 1) {
                    QStringList qualifiedName = signature.at(0).split(QChar(' '));
                    qualifiedName = qualifiedName.last().split(QLatin1String("::"));
                    if (qualifiedName.size() > 1) {
                        QString qualifier = qualifiedName.at(0);
                        int i = 0;
                        while (qualifier.size() > i && !qualifier.at(i).isLetter())
                            qualifier[i++] = QChar(' ');
                        if (i > 0)
                            qualifier = qualifier.simplified();
                        ClassNode *cn = m_qdb->findClassNode(QStringList(qualifier));
                        if (cn && cn->isInternal())
                            report = false;
                    }
                }
                if (report) {
                    location.warning(
                            QStringLiteral("clang couldn't find function when parsing \\fn %1").arg(fnSignature));
                }
            }
        }
    }
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
    return fnNode;
}

void ClangCodeParser::printDiagnostics(const CXTranslationUnit &translationUnit) const
{
    if (!lcQdocClang().isDebugEnabled())
        return;

    static const auto displayOptions = CXDiagnosticDisplayOptions::CXDiagnostic_DisplaySourceLocation
                                     | CXDiagnosticDisplayOptions::CXDiagnostic_DisplayColumn
                                     | CXDiagnosticDisplayOptions::CXDiagnostic_DisplayOption;

    for (unsigned i = 0, numDiagnostics = clang_getNumDiagnostics(translationUnit); i < numDiagnostics; ++i) {
        auto diagnostic = clang_getDiagnostic(translationUnit, i);
        auto formattedDiagnostic = clang_formatDiagnostic(diagnostic, displayOptions);
        qCDebug(lcQdocClang) << clang_getCString(formattedDiagnostic);
        clang_disposeString(formattedDiagnostic);
        clang_disposeDiagnostic(diagnostic);
    }
}

QT_END_NAMESPACE
