// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GENERATOR_H
#define GENERATOR_H

#include "text.h"
#include "utilities.h"
#include "filesystem/fileresolver.h"

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtextstream.h>
#include <optional>

QT_BEGIN_NAMESPACE

typedef QMultiMap<QString, Node *> NodeMultiMap;

class Aggregate;
class CodeMarker;
class ExampleNode;
class FunctionNode;
class Location;
class Node;
class QDocDatabase;

class Generator
{
public:
    enum ListType { Generic, Obsolete };

    enum Addendum {
        Invokable,
        PrivateSignal,
        QmlSignalHandler,
        AssociatedProperties,
        BindableProperty
    };

    Generator(FileResolver& file_resolver);
    virtual ~Generator();

    virtual bool canHandleFormat(const QString &format) { return format == this->format(); }
    virtual QString format() = 0;
    virtual void generateDocs();
    virtual void initializeGenerator();
    virtual void initializeFormat();
    virtual void terminateGenerator();
    virtual QString typeString(const Node *node);

    QString fullDocumentLocation(const Node *node, bool useSubdir = false);
    QString linkForExampleFile(const QString &path, const QString &fileExt = QString());
    static QString exampleFileTitle(const ExampleNode *relative, const QString &fileName);
    static Generator *currentGenerator() { return s_currentGenerator; }
    static Generator *generatorForFormat(const QString &format);
    static void initialize();
    static const QString &outputDir() { return s_outDir; }
    static const QString &outputSubdir() { return s_outSubdir; }
    static void terminate();
    static const QStringList &outputFileNames() { return s_outFileNames; }
    static bool noLinkErrors() { return s_noLinkErrors; }
    static bool autolinkErrors() { return s_autolinkErrors; }
    static QString defaultModuleName() { return s_project; }
    static void resetUseOutputSubdirs() { s_useOutputSubdirs = false; }
    static bool useOutputSubdirs() { return s_useOutputSubdirs; }
    static void setQmlTypeContext(QmlTypeNode *t) { s_qmlTypeContext = t; }
    static QmlTypeNode *qmlTypeContext() { return s_qmlTypeContext; }
    static QString cleanRef(const QString &ref, bool xmlCompliant = false);
    static QString plainCode(const QString &markedCode);
    virtual QString fileBase(const Node *node) const;

protected:
    static QFile *openSubPageFile(const Node *node, const QString &fileName);
    void beginSubPage(const Node *node, const QString &fileName);
    void endSubPage();
    [[nodiscard]] virtual QString fileExtension() const = 0;
    virtual void generateExampleFilePage(const Node *, ResolvedFile, CodeMarker * = nullptr) {}
    virtual void generateAlsoList(const Node *node, CodeMarker *marker);
    virtual void generateAlsoList(const Node *node) { generateAlsoList(node, nullptr); }
    virtual qsizetype generateAtom(const Atom *, const Node *, CodeMarker *) { return 0; }
    virtual qsizetype generateAtom(const Atom *atom, const Node *node)
    {
        return generateAtom(atom, node, nullptr);
    }
    virtual void generateBody(const Node *node, CodeMarker *marker);
    virtual void generateCppReferencePage(Aggregate *, CodeMarker *) {}
    virtual void generateProxyPage(Aggregate *, CodeMarker *) {}
    virtual void generateQmlTypePage(QmlTypeNode *, CodeMarker *) {}
    virtual void generatePageNode(PageNode *, CodeMarker *) {}
    virtual void generateCollectionNode(CollectionNode *, CodeMarker *) {}
    virtual void generateGenericCollectionPage(CollectionNode *, CodeMarker *) {}
    virtual void generateDocumentation(Node *node);
    virtual bool generateText(const Text &text, const Node *relative, CodeMarker *marker);
    virtual bool generateText(const Text &text, const Node *relative)
    {
        return generateText(text, relative, nullptr);
    };
    virtual int skipAtoms(const Atom *atom, Atom::AtomType type) const;

    static bool matchAhead(const Atom *atom, Atom::AtomType expectedAtomType);
    static QString outputPrefix(const Node *node);
    static QString outputSuffix(const Node *node);
    static void supplementAlsoList(const Node *node, QList<Text> &alsoList);
    static QString trimmedTrailing(const QString &string, const QString &prefix,
                                   const QString &suffix);
    void initializeTextOutput();
    QString fileName(const Node *node, const QString &extension = QString()) const;
    QMap<QString, QString> &formattingLeftMap();
    QMap<QString, QString> &formattingRightMap();
    const Atom *generateAtomList(const Atom *atom, const Node *relative, CodeMarker *marker,
                                 bool generate, int &numGeneratedAtoms);
    void generateRequiredLinks(const Node *node, CodeMarker *marker);
    void generateLinkToExample(const ExampleNode *en, CodeMarker *marker,
                               const QString &exampleUrl);
    virtual void generateFileList(const ExampleNode *en, CodeMarker *marker, bool images);
    static QString formatSince(const Node *node);
    void generateSince(const Node *node, CodeMarker *marker);
    void generateNoexceptNote(const Node *node, CodeMarker *marker);
    void generateStatus(const Node *node, CodeMarker *marker);
    virtual void generateAddendum(const Node *node, Addendum type, CodeMarker *marker,
                                  bool generateNote);
    virtual void generateAddendum(const Node *node, Addendum type, CodeMarker *marker)
    {
        generateAddendum(node, type, marker, true);
    };
    void generateThreadSafeness(const Node *node, CodeMarker *marker);
    QStringList getMetadataElements(const Aggregate *inner, const QString &t);
    void generateOverloadedSignal(const Node *node, CodeMarker *marker);
    static QString getOverloadedSignalCode(const Node *node);
    QString indent(int level, const QString &markedCode);
    QTextStream &out();
    QString outFileName();
    bool parseArg(const QString &src, const QString &tag, int *pos, int n, QStringView *contents,
                  QStringView *par1 = nullptr);
    void unknownAtom(const Atom *atom);
    int appendSortedQmlNames(Text &text, const Node *base, const NodeList &subs);

    static bool hasExceptions(const Node *node, NodeList &reentrant, NodeList &threadsafe,
                              NodeList &nonreentrant);

    QString naturalLanguage;
    QString tagFile_;
    QStack<QTextStream *> outStreamStack;

    void appendFullName(Text &text, const Node *apparentNode, const Node *relative,
                        const Node *actualNode = nullptr);
    void appendFullName(Text &text, const Node *apparentNode, const QString &fullName,
                        const Node *actualNode);
    int appendSortedNames(Text &text, const ClassNode *classe,
                          const QList<RelatedClass> &classes);
    void appendSignature(Text &text, const Node *node);
    void signatureList(const NodeList &nodes, const Node *relative, CodeMarker *marker);

    void addImageToCopy(const ExampleNode *en, const ResolvedFile& resolved_file);
    // TODO: This seems to be used as the predicate in std::sort calls.
    // Remove it as it is unneeded.
    // Indeed, it could be replaced by std::less and, furthermore,
    // std::sort already defaults to operator< when no predicate is
    // provided.
    static bool comparePaths(const QString &a, const QString &b) { return (a < b); }

private:
    static Generator *s_currentGenerator;
    static QMap<QString, QMap<QString, QString>> s_fmtLeftMaps;
    static QMap<QString, QMap<QString, QString>> s_fmtRightMaps;
    static QList<Generator *> s_generators;
    static QString s_project;
    static QString s_outDir;
    static QString s_outSubdir;
    static QStringList s_outFileNames;
    static QSet<QString> s_outputFormats;
    static QHash<QString, QString> s_outputPrefixes;
    static QHash<QString, QString> s_outputSuffixes;
    static bool s_noLinkErrors;
    static bool s_autolinkErrors;
    static bool s_redirectDocumentationToDevNull;
    static bool s_useOutputSubdirs;
    static QmlTypeNode *s_qmlTypeContext;

    void generateReimplementsClause(const FunctionNode *fn, CodeMarker *marker);
    static void copyTemplateFiles(const QString &configVar, const QString &subDir);

protected:
    FileResolver& file_resolver;

    QDocDatabase *m_qdb { nullptr };
    bool m_inLink { false };
    bool m_inContents { false };
    bool m_inSectionHeading { false };
    bool m_inTableHeader { false };
    bool m_threeColumnEnumValueTable { true };
    bool m_showInternal { false };
    bool m_quoting { false };
    int m_numTableRows { 0 };
    QString m_link {};
    QString m_sectionNumber {};
};

std::optional<QString> formatStatus(const Node *node, QDocDatabase *qdb);

QT_END_NAMESPACE

#endif
