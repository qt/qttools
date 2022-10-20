/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef GENERATOR_H
#define GENERATOR_H

#include "text.h"
#include "utilities.h"

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtextstream.h>

QT_BEGIN_NAMESPACE

typedef QMultiMap<QString, Node *> NodeMultiMap;
typedef QMap<Node *, NodeMultiMap> ParentMaps;

class Aggregate;
class CodeMarker;
class ExampleNode;
class FunctionNode;
class Location;
class Node;
class QDocDatabase;
class QmlBasicTypeNode;

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

    Generator();
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
    static void augmentImageDirs(QSet<QString> &moreImageDirs);
    static bool noLinkErrors() { return s_noLinkErrors; }
    static bool autolinkErrors() { return s_autolinkErrors; }
    static QString defaultModuleName() { return s_project; }
    static void resetUseOutputSubdirs() { s_useOutputSubdirs = false; }
    static bool useOutputSubdirs() { return s_useOutputSubdirs; }
    static void setQmlTypeContext(QmlTypeNode *t) { s_qmlTypeContext = t; }
    static QmlTypeNode *qmlTypeContext() { return s_qmlTypeContext; }
    static QString cleanRef(const QString &ref);
    static QString plainCode(const QString &markedCode);
    virtual QString fileBase(const Node *node) const;

protected:
    static QFile *openSubPageFile(const Node *node, const QString &fileName);
    void beginFilePage(const Node *node, const QString &fileName);
    void endFilePage() { endSubPage(); } // for symmetry
    void beginSubPage(const Node *node, const QString &fileName);
    void endSubPage();
    [[nodiscard]] virtual QString fileExtension() const = 0;
    virtual void generateExampleFilePage(const Node *, const QString &, CodeMarker *) {}
    virtual void generateExampleFilePage(const Node *node, const QString &str)
    {
        generateExampleFilePage(node, str, nullptr);
    }
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
    virtual void generateQmlBasicTypePage(QmlBasicTypeNode *, CodeMarker *) {}
    virtual void generatePageNode(PageNode *, CodeMarker *) {}
    virtual void generateCollectionNode(CollectionNode *, CodeMarker *) {}
    virtual void generateGenericCollectionPage(CollectionNode *, CodeMarker *) {}
    virtual void generateDocumentation(Node *node);
    virtual void generateMaintainerList(const Aggregate *node, CodeMarker *marker);
    virtual void generateMaintainerList(const Aggregate *node)
    {
        generateMaintainerList(node, nullptr);
    };
    virtual bool generateQmlText(const Text &text, const Node *relative, CodeMarker *marker,
                                 const QString &qmlName);
    virtual bool generateQmlText(const Text &text, const Node *relative)
    {
        return generateQmlText(text, relative, nullptr, QString());
    }
    virtual bool generateText(const Text &text, const Node *relative, CodeMarker *marker);
    virtual bool generateText(const Text &text, const Node *relative)
    {
        return generateText(text, relative, nullptr);
    };
    virtual QString imageFileName(const Node *relative, const QString &fileBase);
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
                  QStringView *par1 = nullptr, bool debug = false);
    void setImageFileExtensions(const QStringList &extensions);
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

    void addImageToCopy(const ExampleNode *en, const QString &file);
    static bool comparePaths(const QString &a, const QString &b) { return (a < b); }

private:
    static Generator *s_currentGenerator;
    static QStringList s_exampleDirs;
    static QStringList s_exampleImgExts;
    static QMap<QString, QMap<QString, QString>> s_fmtLeftMaps;
    static QMap<QString, QMap<QString, QString>> s_fmtRightMaps;
    static QList<Generator *> s_generators;
    static QStringList s_imageDirs;
    static QStringList s_imageFiles;
    static QMap<QString, QStringList> s_imgFileExts;
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

QT_END_NAMESPACE

#endif
