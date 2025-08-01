// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "clangcodeparser.h"
#include "cppcodeparser.h"

#include "access.h"
#include "classnode.h"
#include "config.h"
#include "enumnode.h"
#include "functionnode.h"
#include "genustypes.h"
#include "namespacenode.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "typedefnode.h"
#include "variablenode.h"
#include "sourcefileparser.h"
#include "utilities.h"

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

#include "clang/AST/QualTypeNames.h"
#include "template_declaration.h"

#include <cstdio>

QT_BEGIN_NAMESPACE

struct CompilationIndex {
    CXIndex index = nullptr;

    operator CXIndex() {
        return index;
    }

    ~CompilationIndex() {
        clang_disposeIndex(index);
    }
};

struct TranslationUnit {
    CXTranslationUnit tu = nullptr;

    operator CXTranslationUnit() {
        return tu;
    }

    operator bool() {
        return tu;
    }

    ~TranslationUnit() {
        clang_disposeTranslationUnit(tu);
    }
};

// We're printing diagnostics in ClangCodeParser::printDiagnostics,
// so avoid clang itself printing them.
static const auto kClangDontDisplayDiagnostics = 0;

static CXTranslationUnit_Flags flags_ = static_cast<CXTranslationUnit_Flags>(0);

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

static void printDiagnostics(const CXTranslationUnit &translationUnit)
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


/*!
 * Returns a string representing the name of \a type as if it was
 * referred to at the end of the translation unit that it was parsed
 * from.
 *
 * For example, given the following code:
 *
 * \code
 * namespace foo {
 *    template<typename T>
 *    struct Bar {
 *       using Baz = const T&;
 *
 *       void bam(Baz);
 *    };
 * }
 * \endcode
 *
 * Given a parsed translation unit and an AST node, say \e {decl},
 * representing the parameter declaration of the first argument of \c {bam},
 * calling \c{get_fully_qualified_name(decl->getType(), * decl->getASTContext())}
 * would result in the string \c {foo::Bar<T>::Baz}.
 *
 * This should generally be used every time the stringified
 * representation of a type is acquired as part of parsing with Clang,
 * so as to ensure a consistent behavior and output.
 */
static std::string get_fully_qualified_type_name(clang::QualType type, const clang::ASTContext& declaration_context) {
     return clang::TypeName::getFullyQualifiedName(
        type,
        declaration_context,
        declaration_context.getPrintingPolicy()
    );
}

/*
 * Retrieves expression as written in the original source code.
 *
 * declaration_context should be the ASTContext of the declaration
 * from which the expression was extracted from.
 *
 * If the expression contains a leading equal sign it will be removed.
 *
 * Leading and trailing spaces will be similarly removed from the expression.
 */
static std::string get_expression_as_string(const clang::Expr* expression, const clang::ASTContext& declaration_context) {
    QString default_value = QString::fromStdString(clang::Lexer::getSourceText(
        clang::CharSourceRange::getTokenRange(expression->getSourceRange()),
        declaration_context.getSourceManager(),
        declaration_context.getLangOpts()
    ).str());

    if (default_value.startsWith("="))
        default_value.remove(0, 1);

    default_value = default_value.trimmed();

    return default_value.toStdString();
}

/*
 * Retrieves the default value of the passed in type template parameter as a string.
 *
 * The default value of a type template parameter is always a type,
 * and its stringified representation will be return as the fully
 * qualified version of the type.
 *
 * If the parameter has no default value the empty string will be returned.
 */
static std::string get_default_value_initializer_as_string(const clang::TemplateTypeParmDecl* parameter) {
#if LIBCLANG_VERSION_MAJOR >= 19
    return (parameter && parameter->hasDefaultArgument()) ?
                get_fully_qualified_type_name(parameter->getDefaultArgument().getArgument().getAsType(), parameter->getASTContext()) :
                "";
#else
    return (parameter && parameter->hasDefaultArgument()) ?
                get_fully_qualified_type_name(parameter->getDefaultArgument(), parameter->getASTContext()) :
                "";
#endif

}

/*
 * Retrieves the default value of the passed in non-type template parameter as a string.
 *
 * The default value of a non-type template parameter is an expression
 * and its stringified representation will be return as it was written
 * in the original code.
 *
 * If the parameter as no default value the empty string will be returned.
 */
static std::string get_default_value_initializer_as_string(const clang::NonTypeTemplateParmDecl* parameter) {
#if LIBCLANG_VERSION_MAJOR >= 19
    return (parameter && parameter->hasDefaultArgument()) ?
        get_expression_as_string(parameter->getDefaultArgument().getSourceExpression(), parameter->getASTContext()) : "";
#else
    return (parameter && parameter->hasDefaultArgument()) ?
        get_expression_as_string(parameter->getDefaultArgument(), parameter->getASTContext()) : "";
#endif

}

/*
 * Retrieves the default value of the passed in template template parameter as a string.
 *
 * The default value of a template template parameter is a template
 * name and its stringified representation will be returned as a fully
 * qualified version of that name.
 *
 * If the parameter as no default value the empty string will be returned.
 */
static std::string get_default_value_initializer_as_string(const clang::TemplateTemplateParmDecl* parameter) {
    std::string default_value{};

    if (parameter && parameter->hasDefaultArgument()) {
        const clang::TemplateName template_name = parameter->getDefaultArgument().getArgument().getAsTemplate();

        llvm::raw_string_ostream ss{default_value};
        template_name.print(ss, parameter->getASTContext().getPrintingPolicy(), clang::TemplateName::Qualified::AsWritten);
    }

    return default_value;
}

/*
 * Retrieves the default value of the passed in function parameter as
 * a string.
 *
 * The default value of a function parameter is an expression and its
 * stringified representation will be returned as it was written in
 * the original code.
 *
 * If the parameter as no default value or Clang was not able to yet
 * parse it at this time the empty string will be returned.
 */
static std::string get_default_value_initializer_as_string(const clang::ParmVarDecl* parameter) {
    if (!parameter || !parameter->hasDefaultArg() || parameter->hasUnparsedDefaultArg())
        return "";

    return get_expression_as_string(
        parameter->hasUninstantiatedDefaultArg() ? parameter->getUninstantiatedDefaultArg() : parameter->getDefaultArg(),
        parameter->getASTContext()
    );
}

/*
 * Retrieves the default value of the passed in declaration, based on
 * its concrete type, as a string.
 *
 * If the declaration is a nullptr or the concrete type of the
 * declaration is not a supported one, the returned string will be the
 * empty string.
 */
static std::string get_default_value_initializer_as_string(const clang::NamedDecl* declaration) {
    if (!declaration) return "";

    if (auto type_template_parameter = llvm::dyn_cast<clang::TemplateTypeParmDecl>(declaration))
        return get_default_value_initializer_as_string(type_template_parameter);

    if (auto non_type_template_parameter = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(declaration))
        return get_default_value_initializer_as_string(non_type_template_parameter);

    if (auto template_template_parameter = llvm::dyn_cast<clang::TemplateTemplateParmDecl>(declaration)) {
        return get_default_value_initializer_as_string(template_template_parameter);
    }

    if (auto function_parameter = llvm::dyn_cast<clang::ParmVarDecl>(declaration)) {
        return get_default_value_initializer_as_string(function_parameter);
    }

    return "";
}

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

/*
 * Returns an intermediate representation that models the the given
 * template declaration.
 */
static RelaxedTemplateDeclaration get_template_declaration(const clang::TemplateDecl* template_declaration) {
    assert(template_declaration);

    RelaxedTemplateDeclaration template_declaration_ir{};

    auto template_parameters = template_declaration->getTemplateParameters();
    for (auto template_parameter : template_parameters->asArray()) {
        auto kind{RelaxedTemplateParameter::Kind::TypeTemplateParameter};
        std::string type{};

        if (auto non_type_template_parameter = llvm::dyn_cast<clang::NonTypeTemplateParmDecl>(template_parameter)) {
            kind = RelaxedTemplateParameter::Kind::NonTypeTemplateParameter;
            type = get_fully_qualified_type_name(non_type_template_parameter->getType(), non_type_template_parameter->getASTContext());

            // REMARK: QDoc uses this information to match a user
            // provided documentation (for example from an "\fn"
            // command) with a `Node` that was extracted from the
            // code-base.
            //
            // Due to how QDoc obtains an AST for documentation that
            // is provided by the user, there might be a mismatch in
            // the type of certain non type template parameters.
            //
            // QDoc generally builds a fake out-of-line definition for
            // a callable provided through an "\fn" command, when it
            // needs to match it.
            // In that context, certain type names may be dependent
            // names, while they may not be when the element they
            // represent is extracted from the code-base.
            //
            // This in turn makes their stringified representation
            // different in the two contextes, as a dependent name may
            // require the "typename" keyword to precede it.
            //
            // Since QDoc uses a very simplified model, and it
            // generally doesn't need care about the exact name
            // resolution rules for C++, since it passes by
            // Clang-validated data, we remove the "typename" keyword
            // if it prefixes the type representation, so that it
            // doesn't impact the matching procedure..

            // KLUDGE: Waiting for C++20 to avoid the conversion.
            //         Doesn't really impact performance in a
            //         meaningful way so it can be kept while waiting.
            if (QString::fromStdString(type).startsWith("typename ")) type.erase(0, std::string("typename ").size());
        }

        auto template_template_parameter = llvm::dyn_cast<clang::TemplateTemplateParmDecl>(template_parameter);
        if (template_template_parameter) kind = RelaxedTemplateParameter::Kind::TemplateTemplateParameter;

        template_declaration_ir.parameters.push_back({
            kind,
            template_parameter->isTemplateParameterPack(),
            {
                std::move(type),
                template_parameter->getNameAsString(),
                get_default_value_initializer_as_string(template_parameter)
            },
            (template_template_parameter ?
                std::optional<TemplateDeclarationStorage>(TemplateDeclarationStorage{
                    get_template_declaration(template_template_parameter).parameters
                }) : std::nullopt)
        });
    }

    return template_declaration_ir;
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

    QFile file(QString::fromUtf8(fileName));
    if (file.open(QIODeviceBase::ReadOnly)) { // binary to match clang offsets
        FileCacheEntry entry{std::move(fileName), file.readAll()};
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
        auto conversion_declaration =
                static_cast<const clang::CXXConversionDecl*>(get_cursor_declaration(cursor));

        return QLatin1String("operator ") + QString::fromStdString(get_fully_qualified_type_name(
            conversion_declaration->getConversionType(),
            conversion_declaration->getASTContext()
        ));
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
  Find the node from the QDocDatabase \a qdb that corresponds to the declaration
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
    // Special case; if the cursor represents a template type|non-type|template parameter
    // and its semantic parent is a function, return a pointer to the function node.
    if (p && p->isFunction(Genus::CPP)) {
        switch (kind) {
        case CXCursor_TemplateTypeParameter:
        case CXCursor_NonTypeTemplateParameter:
        case CXCursor_TemplateTemplateParameter:
            return p;
        default:
            break;
        }
    }

    // ...otherwise, the semantic parent must be an Aggregate node.
    if (!p || !p->isAggregate())
        return nullptr;
    auto parent = static_cast<Aggregate *>(p);

    QString name;
    if (clang_Cursor_isAnonymous(cur)) {
        name = Utilities::uniqueIdentifier(
                fromCXSourceLocation(clang_getCursorLocation(cur)),
                QLatin1String("anonymous"));
    } else {
        name = fromCXString(clang_getCursorSpelling(cur));
    }
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
        // Hidden friend functions are recorded under their lexical parent in the database
        if (candidates.isEmpty() && get_cursor_declaration(cur)->getFriendObjectKind() != clang::Decl::FOK_None) {
            if (auto *lexical_parent = findNodeForCursor(qdb, clang_getCursorLexicalParent(cur));
                    lexical_parent && lexical_parent->isAggregate() && lexical_parent != parent) {
                static_cast<Aggregate *>(lexical_parent)->findChildren(functionName(cur), candidates);
            }
        }

        if (candidates.isEmpty())
            return nullptr;

        CXType funcType = clang_getCursorType(cur);
        auto numArg = clang_getNumArgTypes(funcType);
        bool isVariadic = clang_isFunctionTypeVariadic(funcType);
        QVarLengthArray<QString, 20> args;

        std::optional<RelaxedTemplateDeclaration> relaxed_template_declaration{std::nullopt};
        if (kind == CXCursor_FunctionTemplate)
            relaxed_template_declaration = get_template_declaration(
                get_cursor_declaration(cur)->getAsFunction()->getDescribedFunctionTemplate()
            );

        for (Node *candidate : std::as_const(candidates)) {
            if (!candidate->isFunction(Genus::CPP))
                continue;

            auto fn = static_cast<FunctionNode *>(candidate);

            if (!fn->templateDecl() && relaxed_template_declaration)
                continue;

            if (fn->templateDecl() && !relaxed_template_declaration)
                continue;

            if (fn->templateDecl() && relaxed_template_declaration &&
                !are_template_declarations_substitutable(*fn->templateDecl(), *relaxed_template_declaration))
                continue;

            const Parameters &parameters = fn->parameters();

            if (parameters.count() != numArg + isVariadic) {
                // Ignore possible last argument of type QPrivateSignal as it may have been dropped
                if (numArg > 0 && parameters.isPrivateSignal() &&
                        (parameters.isEmpty() || !parameters.last().type().endsWith(
                                QLatin1String("QPrivateSignal")))) {
                    if (parameters.count() != --numArg + isVariadic)
                        continue;
                } else {
                continue;
                }
            }

            if (fn->isConst() != bool(clang_CXXMethod_isConst(cur)))
                continue;

            if (isVariadic && parameters.last().type() != QLatin1String("..."))
                continue;

            if (fn->isRef() != (clang_Type_getCXXRefQualifier(funcType) == CXRefQualifier_LValue))
                continue;

            if (fn->isRefRef() != (clang_Type_getCXXRefQualifier(funcType) == CXRefQualifier_RValue))
                continue;

            auto function_declaration = get_cursor_declaration(cur)->getAsFunction();

            bool different = false;
            for (int i = 0; i < numArg; ++i) {
                CXType argType = clang_getArgType(funcType, i);

                if (args.size() <= i)
                    args.append(QString::fromStdString(get_fully_qualified_type_name(
                        function_declaration->getParamDecl(i)->getOriginalType(),
                        function_declaration->getASTContext()
                    )));

                QString recordedType = parameters.at(i).type();
                QString typeSpelling = args.at(i);

                different = recordedType != typeSpelling;

                // Retry with a canonical type spelling
                if (different && (argType.kind == CXType_Typedef || argType.kind == CXType_Elaborated)) {
                    QStringView canonicalType = parameters.at(i).canonicalType();
                    if (!canonicalType.isEmpty()) {
                        different = canonicalType !=
                            QString::fromStdString(get_fully_qualified_type_name(
                                function_declaration->getParamDecl(i)->getOriginalType().getCanonicalType(),
                                function_declaration->getASTContext()
                            ));
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
    ClangVisitor(QDocDatabase *qdb, const std::set<Config::HeaderFilePath> &allHeaders)
        : qdb_(qdb), parent_(qdb->primaryTreeRoot())
    {
        std::transform(allHeaders.cbegin(), allHeaders.cend(), std::inserter(allHeaders_, allHeaders_.begin()),
                       [](const auto& header_file_path) -> const QString& { return header_file_path.filename; });
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
                isInteresting = allHeaders_.find(fi.fileName()) != allHeaders_.end();
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
    std::set<QString> allHeaders_;
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
                if ((*fnNode)->isFunction(Genus::CPP)) {
                    auto *fn = static_cast<FunctionNode *>(*fnNode);
                    readParameterNamesAndAttributes(fn, cursor);

                    const clang::Decl* declaration = get_cursor_declaration(cursor);
                    assert(declaration);
                    if (const auto function_declaration = declaration->getAsFunction()) {
                        auto declaredReturnType = function_declaration->getDeclaredReturnType();
                        if (llvm::dyn_cast_if_present<clang::AutoType>(declaredReturnType.getTypePtrOrNull()))
                            fn->setDeclaredReturnType(QString::fromStdString(declaredReturnType.getAsString()));
                    }
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

    switch (kind) {
    case CXCursor_TypeAliasTemplateDecl:
    case CXCursor_TypeAliasDecl: {
        const QString aliasName = fromCXString(clang_getCursorSpelling(cursor));
        QString aliasedType;

        const auto *templateDecl = (kind == CXCursor_TypeAliasTemplateDecl)
                                 ? llvm::dyn_cast<clang::TemplateDecl>(get_cursor_declaration(cursor))
                                 : nullptr;

        if (kind == CXCursor_TypeAliasTemplateDecl) {
            // For template aliases, get the underlying TypeAliasDecl from the TemplateDecl
            if (const auto *aliasTemplate = llvm::dyn_cast<clang::TypeAliasTemplateDecl>(templateDecl)) {
                if (const auto *aliasDecl = aliasTemplate->getTemplatedDecl()) {
                    clang::QualType underlyingType = aliasDecl->getUnderlyingType();
                    aliasedType = QString::fromStdString(underlyingType.getAsString());
                }
            }
        } else {
            // For non-template aliases, get the underlying type via C API
            const CXType aliasedCXType = clang_getTypedefDeclUnderlyingType(cursor);
            if (aliasedCXType.kind != CXType_Invalid) {
                aliasedType = fromCXString(clang_getTypeSpelling(aliasedCXType));
            }
        }

        if (!aliasedType.isEmpty()) {
            auto *ta = new TypeAliasNode(parent_, aliasName, aliasedType);
            ta->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
            ta->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));

            if (templateDecl)
                ta->setTemplateDecl(get_template_declaration(templateDecl));
        }
        return CXChildVisit_Continue;
    }
    case CXCursor_StructDecl:
    case CXCursor_UnionDecl:
        if (fromCXString(clang_getCursorSpelling(cursor)).isEmpty()) // anonymous struct or union
            return CXChildVisit_Continue;
        Q_FALLTHROUGH();
    case CXCursor_ClassTemplate:
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

        NodeType type = NodeType::Class;
        if (actualKind == CXCursor_StructDecl)
            type = NodeType::Struct;
        else if (actualKind == CXCursor_UnionDecl)
            type = NodeType::Union;

        auto *classe = new ClassNode(type, semanticParent, className);
        classe->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
        classe->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));

        if (kind == CXCursor_ClassTemplate) {
            auto template_declaration = llvm::dyn_cast<clang::TemplateDecl>(get_cursor_declaration(cursor));
            classe->setTemplateDecl(get_template_declaration(template_declaration));
        }

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

        if (kind == CXCursor_FunctionTemplate) {
            auto template_declaration = get_cursor_declaration(cursor)->getAsFunction()->getDescribedFunctionTemplate();
            fn->setTemplateDecl(get_template_declaration(template_declaration));
        }

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

        if (clang_Cursor_isAnonymous(cursor)) {
            enumTypeName = "anonymous";
            // Generate a unique name to enable auto-tying doc comments in headers
            // to anonymous enum declarations
            if (Config::instance().get(CONFIG_DOCUMENTATIONINHEADERS).asBool())
                enumTypeName = Utilities::uniqueIdentifier(fromCXSourceLocation(clang_getCursorLocation(cursor)), enumTypeName);
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
            en->setAnonymous(clang_Cursor_isAnonymous(cursor));
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

            en->addItem(EnumItem(fromCXString(clang_getCursorSpelling(cur)), std::move(value)));
            return CXChildVisit_Continue;
        });
        return CXChildVisit_Continue;
    }
    case CXCursor_FieldDecl:
    case CXCursor_VarDecl: {
        if (findNodeForCursor(qdb_, cursor)) // Was already parsed, probably in another TU
            return CXChildVisit_Continue;

        auto value_declaration =
            llvm::dyn_cast<clang::ValueDecl>(get_cursor_declaration(cursor));
        assert(value_declaration);

        auto access = fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor));
        auto var = new VariableNode(parent_, fromCXString(clang_getCursorSpelling(cursor)));

        var->setAccess(access);
        var->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
        var->setLeftType(QString::fromStdString(get_fully_qualified_type_name(
            value_declaration->getType(),
            value_declaration->getASTContext()
        )));
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

            if (QString name = fromCXString(clang_getCursorSpelling(cur)); !name.isEmpty())
                parameters[i].setName(name);

            const clang::ParmVarDecl* parameter_declaration = llvm::dyn_cast<const clang::ParmVarDecl>(get_cursor_declaration(cur));
            Q_ASSERT(parameter_declaration);

            std::string default_value = get_default_value_initializer_as_string(parameter_declaration);

            if (!default_value.empty())
                parameters[i].setDefaultValue(QString::fromStdString(default_value));

            ++i;
        }
        return CXChildVisit_Continue;
    });
}

void ClangVisitor::processFunction(FunctionNode *fn, CXCursor cursor)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    CXType funcType = clang_getCursorType(cursor);
    fn->setAccess(fromCX_CXXAccessSpecifier(clang_getCXXAccessSpecifier(cursor)));
    fn->setLocation(fromCXSourceLocation(clang_getCursorLocation(cursor)));
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

    assert(declaration);

    const clang::FunctionDecl* function_declaration = declaration->getAsFunction();

    if (kind == CXCursor_Constructor
        // a constructor template is classified as CXCursor_FunctionTemplate
        || (kind == CXCursor_FunctionTemplate && fn->name() == parent_->name()))
        fn->setMetaness(FunctionNode::Ctor);
    else if (kind == CXCursor_Destructor)
        fn->setMetaness(FunctionNode::Dtor);
    else
        fn->setReturnType(QString::fromStdString(get_fully_qualified_type_name(
            function_declaration->getReturnType(),
            function_declaration->getASTContext()
        )));

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
            const std::string exception_specification_spelling =
                exception_specification.NoexceptExpr ? get_expression_as_string(
                    exception_specification.NoexceptExpr,
                    function_declaration->getASTContext()
                ) : "";

            if (exception_specification_spelling != "false")
                fn->markNoexcept(QString::fromStdString(exception_specification_spelling));
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

    for (clang::ParmVarDecl* const parameter_declaration : function_declaration->parameters()) {
        clang::QualType parameter_type = parameter_declaration->getOriginalType();

        parameters.append(QString::fromStdString(get_fully_qualified_type_name(
            parameter_type,
            parameter_declaration->getASTContext()
        )));

        if (!parameter_type.isCanonical())
            parameters.last().setCanonicalType(QString::fromStdString(get_fully_qualified_type_name(
                parameter_type.getCanonicalType(),
                parameter_declaration->getASTContext()
            )));
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
    if (node && node->isFunction(Genus::CPP))
        readParameterNamesAndAttributes(static_cast<FunctionNode *>(node), *decl_it);
    return node;
}

ClangCodeParser::ClangCodeParser(
    QDocDatabase* qdb,
    Config& config,
    const std::vector<QByteArray>& include_paths,
    const QList<QByteArray>& defines,
    std::optional<std::reference_wrapper<const PCHFile>> pch
) : m_qdb{qdb},
    m_includePaths{include_paths},
    m_defines{defines},
    m_pch{pch}
{
    m_allHeaders = config.getHeaderFiles();
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
    "-xc++"
};

/*!
  Load the default arguments and the defines into \a args.
  Clear \a args first.
 */
void getDefaultArgs(const QList<QByteArray>& defines, std::vector<const char*>& args)
{
    args.clear();
    args.insert(args.begin(), std::begin(defaultArgs_), std::end(defaultArgs_));

    // Add the defines from the qdocconf file.
    for (const auto &p : std::as_const(defines))
        args.push_back(p.constData());
}

static QList<QByteArray> includePathsFromHeaders(const std::set<Config::HeaderFilePath> &allHeaders)
{
    QList<QByteArray> result;
    for (const auto& [header_path, _] : allHeaders) {
        const QByteArray path = "-I" + header_path.toLatin1();
        const QByteArray parent =
                "-I" + QDir::cleanPath(header_path + QLatin1String("/../")).toLatin1();
    }

    return result;
}

/*!
  Load the include paths into \a moreArgs. If no include paths
  were provided, try to guess reasonable include paths.
 */
void getMoreArgs(
    const std::vector<QByteArray>& include_paths,
    const std::set<Config::HeaderFilePath>& all_headers,
    std::vector<const char*>& args
) {
    if (include_paths.empty()) {
        /*
          The include paths provided are inadequate. Make a list
          of reasonable places to look for include files and use
          that list instead.
         */
        qCWarning(lcQdoc) << "No include paths passed to qdoc; guessing reasonable include paths";

        QString basicIncludeDir = QDir::cleanPath(QString(Config::installDir + "/../include"));
        args.emplace_back(QByteArray("-I" + basicIncludeDir.toLatin1()).constData());

        auto include_paths_from_headers = includePathsFromHeaders(all_headers);
        args.insert(args.end(), include_paths_from_headers.begin(), include_paths_from_headers.end());
    } else {
        std::copy(include_paths.begin(), include_paths.end(), std::back_inserter(args));
    }
}

/*!
  Building the PCH must be possible when there are no .cpp
  files, so it is moved here to its own member function, and
  it is called after the list of header files is complete.
 */
std::optional<PCHFile> buildPCH(
    QDocDatabase* qdb,
    QString module_header,
    const std::set<Config::HeaderFilePath>& all_headers,
    const std::vector<QByteArray>& include_paths,
    const QList<QByteArray>& defines
) {
    static std::vector<const char*> arguments{};

    if (module_header.isEmpty()) return std::nullopt;

    getDefaultArgs(defines, arguments);
    getMoreArgs(include_paths, all_headers, arguments);

    flags_ = static_cast<CXTranslationUnit_Flags>(CXTranslationUnit_Incomplete
                                                  | CXTranslationUnit_SkipFunctionBodies
                                                  | CXTranslationUnit_KeepGoing);

    CompilationIndex index{ clang_createIndex(1, kClangDontDisplayDiagnostics) };

    QTemporaryDir pch_directory{QDir::tempPath() + QLatin1String("/qdoc_pch")};
    if (!pch_directory.isValid()) return std::nullopt;

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
    auto it = std::find_if(include_paths.begin(), include_paths.end(),
                            FindPredicate(candidate, module, FindPredicate::Module));
    if (it == include_paths.end())
        it = std::find_if(include_paths.begin(), include_paths.end(),
                            FindPredicate(candidate, module, FindPredicate::Any));
    if (it != include_paths.end())
        header = std::move(candidate);

    if (header.isEmpty()) {
        qWarning() << "(qdoc) Could not find the module header in include paths for module"
                    << module << "  (include paths: " << include_paths << ")";
        qWarning() << "       Artificial module header built from header dirs in qdocconf "
                        "file";
    }
    arguments.push_back("-xc++");

    TranslationUnit tu;

    QString tmpHeader = pch_directory.path() + "/" + module;
    if (QFile tmpHeaderFile(tmpHeader); tmpHeaderFile.open(QIODevice::Text | QIODevice::WriteOnly)) {
        QTextStream out(&tmpHeaderFile);
        if (header.isEmpty()) {
            for (const auto& [header_path, header_name] : all_headers) {
                if (!header_name.endsWith(QLatin1String("_p.h"))
                    && !header_name.startsWith(QLatin1String("moc_"))) {
                    QString line = QLatin1String("#include \"") + header_path
                            + QLatin1String("/") + header_name + QLatin1String("\"");
                    out << line << "\n";

                }
            }
        } else {
            QFileInfo headerFile(header);
            if (!headerFile.exists()) {
                qWarning() << "Could not find module header file" << header;
                return std::nullopt;
            }
            out << QLatin1String("#include \"") + header + QLatin1String("\"");
        }
    }

    CXErrorCode err =
            clang_parseTranslationUnit2(index, tmpHeader.toLatin1().data(), arguments.data(),
                                        static_cast<int>(arguments.size()), nullptr, 0,
                                        flags_ | CXTranslationUnit_ForSerialization, &tu.tu);
    qCDebug(lcQdoc) << __FUNCTION__ << "clang_parseTranslationUnit2(" << tmpHeader << arguments
                    << ") returns" << err;

    printDiagnostics(tu);

    if (err || !tu) {
        qCCritical(lcQdoc) << "Could not create PCH file for " << module_header;
        return std::nullopt;
    }

    QByteArray pch_name = pch_directory.path().toUtf8() + "/" + module + ".pch";
    auto error = clang_saveTranslationUnit(tu, pch_name.constData(),
                                            clang_defaultSaveOptions(tu));
    if (error) {
        qCCritical(lcQdoc) << "Could not save PCH file for" << module_header;
        return std::nullopt;
    }

    // Visit the header now, as token from pre-compiled header won't be visited
    // later
    CXCursor cur = clang_getTranslationUnitCursor(tu);
    ClangVisitor visitor(qdb, all_headers);
    visitor.visitChildren(cur);
    qCDebug(lcQdoc) << "PCH built and visited for" << module_header;

    return std::make_optional(PCHFile{std::move(pch_directory), std::move(pch_name)});
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

  If parsing C++ header file as source, do not use the precompiled
  header as the source file itself is likely already included in the
  PCH and therefore interferes visiting the TU's children.
 */
ParsedCppFileIR ClangCodeParser::parse_cpp_file(const QString &filePath)
{
    flags_ = static_cast<CXTranslationUnit_Flags>(CXTranslationUnit_Incomplete
                                                  | CXTranslationUnit_SkipFunctionBodies
                                                  | CXTranslationUnit_KeepGoing);

    CompilationIndex index{ clang_createIndex(1, kClangDontDisplayDiagnostics) };

    getDefaultArgs(m_defines, m_args);
    if (m_pch && !filePath.endsWith(".mm")
            && !std::holds_alternative<CppHeaderSourceFile>(tag_source_file(filePath).second)) {
        m_args.push_back("-w");
        m_args.push_back("-include-pch");
        m_args.push_back((*m_pch).get().name.constData());
    }
    getMoreArgs(m_includePaths, m_allHeaders, m_args);

    TranslationUnit tu;
    CXErrorCode err =
            clang_parseTranslationUnit2(index, filePath.toLocal8Bit(), m_args.data(),
                                        static_cast<int>(m_args.size()), nullptr, 0, flags_, &tu.tu);
    qCDebug(lcQdoc) << __FUNCTION__ << "clang_parseTranslationUnit2(" << filePath << m_args
                    << ") returns" << err;
    printDiagnostics(tu);

    if (err || !tu) {
        qWarning() << "(qdoc) Could not parse source file" << filePath << " error code:" << err;
        return {};
    }

    ParsedCppFileIR parse_result{};

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
        if (hasTooManyTopics(doc))
            continue;

        if (doc.topicsUsed().isEmpty()) {
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
                parse_result.tied.emplace_back(TiedDocumentation{doc, n});
            } else if (CodeParser::isWorthWarningAbout(doc)) {
                bool future = false;
                if (doc.metaCommandsUsed().contains(COMMAND_SINCE)) {
                    QString sinceVersion = doc.metaCommandArgs(COMMAND_SINCE).at(0).first;
                    if (getUnpatchedVersion(std::move(sinceVersion)) >
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
            parse_result.untied.emplace_back(UntiedDocumentation{doc, QStringList()});

            CXCursor cur = clang_getCursor(tu, commentLoc);
            while (true) {
                CXCursorKind kind = clang_getCursorKind(cur);
                if (clang_isTranslationUnit(kind) || clang_isInvalid(kind))
                    break;
                if (kind == CXCursor_Namespace) {
                    parse_result.untied.back().context << fromCXString(clang_getCursorSpelling(cur));
                }
                cur = clang_getCursorLexicalParent(cur);
            }
        }
    }

    clang_disposeTokens(tu, tokens, numTokens);
    m_namespaceScope.clear();
    s_fn.clear();

    return parse_result;
}

/*!
  Use clang to parse the function signature from a function
  command. \a location is used for reporting errors. \a fnSignature
  is the string to parse. It is always a function decl.
  \a idTag is the optional bracketed argument passed to \\fn, or
  an empty string.
  \a context is a string list representing the scope (namespaces)
  under which the function is declared.

  Returns a variant that's either a Node instance tied to the
  function declaration, or a parsing failure for later processing.
 */
std::variant<Node*, FnMatchError> FnCommandParser::operator()(const Location &location, const QString &fnSignature,
                                  const QString &idTag, QStringList context)
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

    CompilationIndex index{ clang_createIndex(1, kClangDontDisplayDiagnostics) };

    getDefaultArgs(m_defines, m_args);

    if (m_pch) {
        m_args.push_back("-w");
        m_args.push_back("-include-pch");
        m_args.push_back((*m_pch).get().name.constData());
    }

    TranslationUnit tu;
    QByteArray s_fn{};
    for (const auto &ns : std::as_const(context))
        s_fn.prepend("namespace " + ns.toUtf8() + " {");
    s_fn += fnSignature.toUtf8();
    if (!s_fn.endsWith(";"))
        s_fn += "{ }";
    s_fn.append(context.size(), '}');

    const char *dummyFileName = fnDummyFileName;
    CXUnsavedFile unsavedFile { dummyFileName, s_fn.constData(),
                                static_cast<unsigned long>(s_fn.size()) };
    CXErrorCode err = clang_parseTranslationUnit2(index, dummyFileName, m_args.data(),
                                                  int(m_args.size()), &unsavedFile, 1, flags, &tu.tu);
    qCDebug(lcQdoc) << __FUNCTION__ << "clang_parseTranslationUnit2(" << dummyFileName << m_args
                    << ") returns" << err;
    printDiagnostics(tu);
    if (err || !tu) {
        location.error(QStringLiteral("clang could not parse \\fn %1").arg(fnSignature));
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

        if (!fnNode) {
            unsigned diagnosticCount = clang_getNumDiagnostics(tu);
            const auto &config = Config::instance();
            if (diagnosticCount > 0 && (!config.preparing() || config.singleExec())) {
               return FnMatchError{ fnSignature, location };
            }
        }
    }
    return fnNode;
}

QT_END_NAMESPACE
