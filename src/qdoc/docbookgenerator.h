// Copyright (C) 2019 Thibaut Cuvelier
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DOCBOOKGENERATOR_H
#define DOCBOOKGENERATOR_H

#include "codemarker.h"
#include "config.h"
#include "xmlgenerator.h"
#include "filesystem/fileresolver.hpp"

#include <QtCore/qhash.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class Aggregate;
class ExampleNode;
class FunctionNode;

class DocBookGenerator : public XmlGenerator
{
public:
    explicit DocBookGenerator(FileResolver& file_resolver);

    void initializeGenerator() override;
    QString format() override;

protected:
    [[nodiscard]] QString fileExtension() const override;
    void generateDocumentation(Node *node) override;
    using Generator::generateCppReferencePage;
    void generateCppReferencePage(Node *node);
    using Generator::generatePageNode;
    void generatePageNode(PageNode *pn);
    using Generator::generateQmlTypePage;
    void generateQmlTypePage(QmlTypeNode *qcn);
    using Generator::generateQmlBasicTypePage;
    void generateQmlBasicTypePage(QmlValueTypeNode *qbtn);
    using Generator::generateCollectionNode;
    void generateCollectionNode(CollectionNode *cn);
    using Generator::generateGenericCollectionPage;
    void generateGenericCollectionPage(CollectionNode *cn);
    using Generator::generateProxyPage;
    void generateProxyPage(Aggregate *aggregate);

    void generateList(const Node *relative, const QString &selector);
    void generateHeader(const QString &title, const QString &subtitle, const Node *node);
    void closeTextSections();
    void generateFooter();
    void generateDocBookSynopsis(const Node *node);
    void generateRequisites(const Aggregate *inner);
    void generateQmlRequisites(const QmlTypeNode *qcn);
    void generateSortedNames(const ClassNode *cn, const QList<RelatedClass> &rc);
    void generateSortedQmlNames(const Node *base, const NodeList &subs);
    bool generateStatus(const Node *node);
    bool generateThreadSafeness(const Node *node);
    bool generateSince(const Node *node);
    void generateAddendum(const Node *node, Generator::Addendum type, CodeMarker *marker,
                          bool generateNote) override;
    using Generator::generateBody;
    void generateBody(const Node *node);

    bool generateText(const Text &text, const Node *relative) override;
    const Atom *generateAtomList(const Atom *atom, const Node *relative, bool generate,
                                 int &numAtoms);
    qsizetype generateAtom(const Atom *atom, const Node *relative) override;

private:
    QXmlStreamWriter *startDocument(const Node *node);
    QXmlStreamWriter *startDocument(const ExampleNode *en, const QString &file);
    QXmlStreamWriter *startGenericDocument(const Node *node, const QString &fileName);
    void endDocument();

    void generateAnnotatedList(const Node *relative, const NodeList &nodeList,
                               const QString &selector);
    void generateAnnotatedLists(const Node *relative, const NodeMultiMap &nmm,
                                const QString &selector);
    void generateCompactList(ListType listType, const Node *relative, const NodeMultiMap &nmm,
                             const QString &commonPrefix, const QString &selector);
    using Generator::generateFileList;
    void generateFileList(const ExampleNode *en, bool images);
    void generateObsoleteMembers(const Sections &sections);
    void generateObsoleteQmlMembers(const Sections &sections);
    void generateSectionList(const Section &section, const Node *relative,
                             Section::Status status = Section::Active);
    void generateSectionInheritedList(const Section &section, const Node *relative);
    void generateSynopsisName(const Node *node, const Node *relative, bool generateNameLink);
    void generateParameter(const Parameter &parameter, const Node *relative, bool generateExtra,
                           bool generateType);
    void generateSynopsis(const Node *node, const Node *relative, Section::Style style);
    void generateEnumValue(const QString &enumValue, const Node *relative);
    void generateDetailedMember(const Node *node, const PageNode *relative);
    void generateDetailedQmlMember(Node *node, const Aggregate *relative);

    void generateFullName(const Node *node, const Node *relative);
    void generateFullName(const Node *apparentNode, const QString &fullName,
                          const Node *actualNode);
    void generateBrief(const Node *node);
    void generateAlsoList(const Node *node) override;
    void generateSignatureList(const NodeList &nodes);
    void generateMaintainerList(const Aggregate *node) override;
    void generateReimplementsClause(const FunctionNode *fn);
    void generateClassHierarchy(const Node *relative, NodeMultiMap &classMap);
    void generateFunctionIndex(const Node *relative);
    void generateLegaleseList(const Node *relative);
    void generateExampleFilePage(const Node *en, ResolvedFile resolved_file, CodeMarker* = nullptr) override;
    void generateOverloadedSignal(const Node *node);
    bool generateQmlText(const Text &text, const Node *relative) override;
    void generateRequiredLinks(const Node *node);
    void generateLinkToExample(const ExampleNode *en, const QString &baseUrl);

    void typified(const QString &string, const Node *relative, bool trailingSpace = false,
                  bool generateType = true);
    void generateLink(const Atom *atom);
    void beginLink(const QString &link, const Node *node, const Node *relative);
    void endLink();
    void writeXmlId(const QString &id);
    void writeXmlId(const Node *node);
    inline void newLine();
    void startSectionBegin(const QString &id = "");
    void startSectionBegin(const Node *node);
    void startSectionEnd();
    void startSection(const QString &id, const QString &title);
    void startSection(const Node *node, const QString &title);
    void startSection(const QString &title);
    void endSection();
    void writeAnchor(const QString &id);
    void generateSimpleLink(const QString &href, const QString &text);
    void generateStartRequisite(const QString &description);
    void generateEndRequisite();
    void generateRequisite(const QString &description, const QString &value);
    void generateCMakeRequisite(const QStringList &values);
    void generateSynopsisInfo(const QString &key, const QString &value);
    void generateModifier(const QString &value);

    bool m_inListItemLineOpen {};
    int currentSectionLevel {};
    QStack<int> sectionLevels {};
    QString m_qflagsHref {};
    bool m_inTeletype { false };

    QString m_project {};
    QString m_projectDescription {};
    QString m_naturalLanguage {};
    QString m_buildVersion {};
    QXmlStreamWriter *m_writer { nullptr };

    Config *m_config { nullptr };
};

QT_END_NAMESPACE

#endif
