// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HTMLGENERATOR_H
#define HTMLGENERATOR_H

#include "codemarker.h"
#include "genustypes.h"
#include "xmlgenerator.h"
#include "filesystem/fileresolver.h"

#include <QtCore/qhash.h>
#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

class Aggregate;
class Config;
class ExampleNode;
class HelpProjectWriter;
class ManifestWriter;

class HtmlGenerator : public XmlGenerator
{
public:
    HtmlGenerator(FileResolver& file_resolver);
    ~HtmlGenerator() override;

    void initializeGenerator() override;
    void terminateGenerator() override;
    QString format() override;
    void generateDocs() override;

    QString protectEnc(const QString &string);
    static QString protect(const QString &string);

protected:
    void generateExampleFilePage(const Node *en, ResolvedFile resolved_file, CodeMarker *marker) override;
    qsizetype generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker) override;
    void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) override;
    void generateProxyPage(Aggregate *aggregate, CodeMarker *marker) override;
    void generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker) override;
    void generatePageNode(PageNode *pn, CodeMarker *marker) override;
    void generateCollectionNode(CollectionNode *cn, CodeMarker *marker) override;
    void generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker) override;
    [[nodiscard]] QString fileExtension() const override;

private:
    enum SubTitleSize { SmallSubTitle, LargeSubTitle };
    enum ExtractionMarkType { BriefMark, DetailedDescriptionMark, MemberMark, EndMark };

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
                                 QList<Section> *sections = nullptr);
    void generateSidebar();
    QString generateAllMembersFile(const Section &section, CodeMarker *marker);
    QString generateAllQmlMembersFile(const Sections &sections, CodeMarker *marker);
    QString generateObsoleteMembersFile(const Sections &sections, CodeMarker *marker);
    QString generateObsoleteQmlMembersFile(const Sections &sections, CodeMarker *marker);
    void generateClassHierarchy(const Node *relative, NodeMultiMap &classMap);
    void generateAnnotatedLists(const Node *relative, CodeMarker *marker,
                                const NodeMultiMap &nodeMap);
    void generateAnnotatedList(const Node *relative, CodeMarker *marker, const NodeList &nodes,
                               Qt::SortOrder sortOrder = Qt::AscendingOrder);
    void generateCompactList(ListType listType, const Node *relative, const NodeMultiMap &classMap,
                             bool includeAlphabet, const QString &commonPrefix);
    void generateFunctionIndex(const Node *relative);
    void generateLegaleseList(const Node *relative, CodeMarker *marker);
    bool generateGroupList(CollectionNode *cn, Qt::SortOrder sortOrder = Qt::AscendingOrder);
    void generateList(const Node *relative, CodeMarker *marker, const QString &selector,
                      Qt::SortOrder sortOrder = Qt::AscendingOrder);
    void generateSectionList(const Section &section, const Node *relative, CodeMarker *marker,
                             bool useObsoloteMembers = false);
    void generateQmlSummary(const NodeVector &members, const Node *relative, CodeMarker *marker);
    void generateQmlItem(const Node *node, const Node *relative, CodeMarker *marker, bool summary);
    void generateDetailedQmlMember(Node *node, const Aggregate *relative, CodeMarker *marker);

    void generateSection(const NodeVector &nv, const Node *relative, CodeMarker *marker);
    void generateSynopsis(const Node *node, const Node *relative, CodeMarker *marker,
                          Section::Style style, bool alignNames = false);
    void generateSectionInheritedList(const Section &section, const Node *relative);
    QString highlightedCode(const QString &markedCode, const Node *relative,
                            bool alignNames = false, Genus genus = Genus::DontCare);

    void generateFullName(const Node *apparentNode, const Node *relative,
                          const Node *actualNode = nullptr);
    void generateSourceLink(const Node *node);
    void generateDetailedMember(const Node *node, const PageNode *relative, CodeMarker *marker);
    void generateLink(const Atom *atom);

    QString fileBase(const Node *node) const override;
    QString fileName(const Node *node);

    void beginLink(const QString &link);
    void beginLink(const QString &link, const Node *node, const Node *relative);
    void endLink();
    void generateExtractionMark(const Node *node, ExtractionMarkType markType);
    void addIncludeFileToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites, Text& text,
                              const QString &headerText);
    void addSinceToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites, Text *text,
                       const QString &sinceText) const;
    void addStatusToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites, Text &text,
                        const QString &statusText) const;
    void addCMakeInfoToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites, Text *text,
                           const QString &CMakeInfo) const;
    void addQtVariableToMap(const Aggregate *aggregate, QMap<QString, Text> &requisites, Text *text,
                            const QString &qtVariableText) const;
    void addQmlNativeTypesToMap(QMap<QString, Text> &requisites, Text *text,
                                const QString &nativeTypeText,
                                ClassNode *classe) const;
    void addInheritsToMap(QMap<QString, Text> &requisites, Text *text, const QString &inheritsText,
                          ClassNode *classe);
    void addInheritedByToMap(QMap<QString, Text> &requisites, Text *text,
                             const QString &inheritedBytext, ClassNode *classe);
    void generateTheTable(const QStringList &requisiteOrder, const QMap<QString, Text> &requisites,
                          const Aggregate *aggregate,
                          CodeMarker *marker);
    inline void openUnorderedList();
    inline void closeUnorderedList();

    QString groupReferenceText(PageNode* node);

    static bool s_inUnorderedList;

    int m_codeIndent { 0 };
    QString m_codePrefix {};
    QString m_codeSuffix {};
    HelpProjectWriter *m_helpProjectWriter { nullptr };
    ManifestWriter *m_manifestWriter { nullptr };
    QString m_headerScripts {};
    QString m_headerStyles {};
    QString m_endHeader {};
    QString m_postHeader {};
    QString m_postPostHeader {};
    QString m_prologue {};
    QString m_footer {};
    QString m_address {};
    bool m_noNavigationBar { false };
    QString m_project {};
    QString m_productName {};
    QString m_projectDescription {};
    QString m_projectUrl {};
    QString m_navigationLinks {};
    QString m_navigationSeparator {};
    QString m_homepage {};
    QString m_hometitle {};
    QString m_landingpage {};
    QString m_landingtitle {};
    QString m_cppclassespage {};
    QString m_cppclassestitle {};
    QString m_qmltypespage {};
    QString m_qmltypestitle {};
    QString m_trademarkspage {};
    QString m_buildversion {};
    QString m_qflagsHref {};
    int tocDepth {};

    Config *config { nullptr };

};

#define HTMLGENERATOR_ADDRESS "address"
#define HTMLGENERATOR_FOOTER "footer"
#define HTMLGENERATOR_POSTHEADER "postheader"
#define HTMLGENERATOR_POSTPOSTHEADER "postpostheader"
#define HTMLGENERATOR_PROLOGUE "prologue"
#define HTMLGENERATOR_NONAVIGATIONBAR "nonavigationbar"
#define HTMLGENERATOR_NAVIGATIONSEPARATOR "navigationseparator"
#define HTMLGENERATOR_TOCDEPTH "tocdepth"

QT_END_NAMESPACE

#endif
