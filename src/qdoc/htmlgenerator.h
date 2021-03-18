/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

/*
  htmlgenerator.h
*/

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include "codemarker.h"
#include "xmlgenerator.h"

#include <QtCore/qhash.h>
#include <QtCore/qregexp.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class Config;
class HelpProjectWriter;

class HtmlGenerator : public XmlGenerator
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::HtmlGenerator)

public:
    HtmlGenerator();
    ~HtmlGenerator() override;

    void initializeGenerator() override;
    void terminateGenerator() override;
    QString format() override;
    void generateDocs() override;
    void generateManifestFiles();

    QString protectEnc(const QString &string);
    static QString protect(const QString &string, const QString &encoding = "ISO-8859-1");

protected:
    void generateQAPage() override;
    void generateExampleFilePage(const Node *en, const QString &file, CodeMarker *marker) override;
    QString generateLinksToLinksPage(const QString &module, CodeMarker *marker);
    QString generateLinksToBrokenLinksPage(CodeMarker *marker, int &count);
    virtual int generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker) override;
    void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) override;
    void generateProxyPage(Aggregate *aggregate, CodeMarker *marker) override;
    void generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker) override;
    void generateQmlBasicTypePage(QmlBasicTypeNode *qbtn, CodeMarker *marker) override;
    void generatePageNode(PageNode *pn, CodeMarker *marker) override;
    void generateCollectionNode(CollectionNode *cn, CodeMarker *marker) override;
    void generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker) override;
    QString fileExtension() const override;

    void generateManifestFile(const QString &manifest, const QString &element);
    void readManifestMetaContent();
    void generateKeywordAnchors(const Node *node);

private:
    enum SubTitleSize { SmallSubTitle, LargeSubTitle };
    enum ExtractionMarkType { BriefMark, DetailedDescriptionMark, MemberMark, EndMark };

    struct ManifestMetaFilter
    {
        QSet<QString> names;
        QSet<QString> attributes;
        QSet<QString> tags;
    };

    QString retrieveInstallPath(const ExampleNode *exampleNode);
    void generateNavigationBar(const QString &title, const Node *node, CodeMarker *marker,
                               const QString &buildversion, bool tableItems = false);
    void generateHeader(const QString &title, const Node *node = nullptr,
                        CodeMarker *marker = nullptr);
    void generateTitle(const QString &title, const Text &subTitle, SubTitleSize subTitleSize,
                       const Node *relative, CodeMarker *marker);
    void generateFooter(const Node *node = nullptr);
    void generateRequisites(Aggregate *inner, CodeMarker *marker);
    void generateQmlRequisites(QmlTypeNode *qcn, CodeMarker *marker);
    void generateBrief(const Node *node, CodeMarker *marker, const Node *relative = nullptr,
                       bool addLink = true);
    void generateTableOfContents(const Node *node, CodeMarker *marker,
                                 QVector<Section> *sections = nullptr);
    void generateSidebar();
    QString generateAllMembersFile(const Section &section, CodeMarker *marker);
    QString generateAllQmlMembersFile(const Sections &sections, CodeMarker *marker);
    QString generateObsoleteMembersFile(const Sections &sections, CodeMarker *marker);
    QString generateObsoleteQmlMembersFile(const Sections &sections, CodeMarker *marker);
    void generateClassHierarchy(const Node *relative, NodeMap &classMap);
    void generateAnnotatedList(const Node *relative, CodeMarker *marker,
                               const NodeMultiMap &nodeMap);
    void generateAnnotatedLists(const Node *relative, CodeMarker *marker,
                                const NodeMultiMap &nodeMap);
    void generateAnnotatedList(const Node *relative, CodeMarker *marker, const NodeList &nodes);
    void generateCompactList(ListType listType, const Node *relative, const NodeMultiMap &classMap,
                             bool includeAlphabet, QString commonPrefix);
    void generateFunctionIndex(const Node *relative);
    void generateLegaleseList(const Node *relative, CodeMarker *marker);
    bool generateGroupList(CollectionNode *cn);
    void generateList(const Node *relative, CodeMarker *marker, const QString &selector);
    void generateSectionList(const Section &section, const Node *relative, CodeMarker *marker,
                             Section::Status = Section::Active);
    void generateQmlSummary(const NodeVector &members, const Node *relative, CodeMarker *marker);
    void generateQmlItem(const Node *node, const Node *relative, CodeMarker *marker, bool summary);
    void generateDetailedQmlMember(Node *node, const Aggregate *relative, CodeMarker *marker);
    void generateQmlInherits(QmlTypeNode *qcn, CodeMarker *marker) override;
    void generateQmlInstantiates(QmlTypeNode *qcn, CodeMarker *marker);
    void generateInstantiatedBy(ClassNode *cn, CodeMarker *marker);

    void generateSection(const NodeVector &nv, const Node *relative, CodeMarker *marker);
    void generateSynopsis(const Node *node, const Node *relative, CodeMarker *marker,
                          Section::Style style, bool alignNames = false,
                          const QString *prefix = nullptr);
    void generateSectionInheritedList(const Section &section, const Node *relative);
    QString highlightedCode(const QString &markedCode, const Node *relative,
                            bool alignNames = false, Node::Genus genus = Node::DontCare);

    void generateFullName(const Node *apparentNode, const Node *relative,
                          const Node *actualNode = nullptr);
    void generateDetailedMember(const Node *node, const PageNode *relative, CodeMarker *marker);
    void generateLink(const Atom *atom, CodeMarker *marker);

    QString fileBase(const Node *node) const override;
    QString fileName(const Node *node);
#ifdef GENERATE_MAC_REFS
    void generateMacRef(const Node *node, CodeMarker *marker);
#endif
    void beginLink(const QString &link);
    void beginLink(const QString &link, const Node *node, const Node *relative);
    void endLink();
    void generateExtractionMark(const Node *node, ExtractionMarkType markType);
    void reportOrphans(const Aggregate *parent);

    QXmlStreamWriter &xmlWriter();

    int codeIndent;
    QString codePrefix;
    QString codeSuffix;
    HelpProjectWriter *helpProjectWriter;
    bool inObsoleteLink;
    QRegExp funcLeftParen;
    QString style;
    QString headerScripts;
    QString headerStyles;
    QString endHeader;
    QString postHeader;
    QString postPostHeader;
    QString prologue;
    QString footer;
    QString address;
    bool pleaseGenerateMacRef;
    bool noNavigationBar;
    QString project;
    QString projectDescription;
    QString projectUrl;
    QString navigationLinks;
    QString navigationSeparator;
    QString manifestDir;
    QString examplesPath;
    QStringList stylesheets;
    QStringList customHeadElements;
    bool obsoleteLinks;
    QStack<QXmlStreamWriter *> xmlWriterStack;
    static int id;
    QVector<ManifestMetaFilter> manifestMetaContent;
    QString homepage;
    QString hometitle;
    QString landingpage;
    QString landingtitle;
    QString cppclassespage;
    QString cppclassestitle;
    QString qmltypespage;
    QString qmltypestitle;
    QString buildversion;
    QString qflagsHref_;
    int tocDepth;

    Config *config;

public:
    static bool debugging_on;
    static QString divNavTop;
};

#define HTMLGENERATOR_ADDRESS "address"
#define HTMLGENERATOR_FOOTER "footer"
#define HTMLGENERATOR_GENERATEMACREFS "generatemacrefs" // ### document me
#define HTMLGENERATOR_POSTHEADER "postheader"
#define HTMLGENERATOR_POSTPOSTHEADER "postpostheader"
#define HTMLGENERATOR_PROLOGUE "prologue"
#define HTMLGENERATOR_NONAVIGATIONBAR "nonavigationbar"
#define HTMLGENERATOR_NAVIGATIONSEPARATOR "navigationseparator"
#define HTMLGENERATOR_NOSUBDIRS "nosubdirs"
#define HTMLGENERATOR_TOCDEPTH "tocdepth"

QT_END_NAMESPACE

#endif
