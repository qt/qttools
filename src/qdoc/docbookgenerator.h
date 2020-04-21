/****************************************************************************
**
** Copyright (C) 2019 Thibaut Cuvelier
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
  docbookgenerator.h
*/

#ifndef DOCBOOKGENERATOR_H
#define DOCBOOKGENERATOR_H

#include "codemarker.h"
#include "config.h"
#include "xmlgenerator.h"

#include <QtCore/qhash.h>
#include <QtCore/qregexp.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class DocBookGenerator : public XmlGenerator
{
public:
    explicit DocBookGenerator() = default;

    void initializeGenerator() override;
    QString format() override;

protected:
    QString fileExtension() const override;
    void generateDocumentation(Node *node) override;
    using Generator::generateCppReferencePage;
    void generateCppReferencePage(Node *node);
    using Generator::generatePageNode;
    void generatePageNode(PageNode *pn);
    using Generator::generateQmlTypePage;
    void generateQmlTypePage(QmlTypeNode *qcn);
    using Generator::generateQmlBasicTypePage;
    void generateQmlBasicTypePage(QmlBasicTypeNode *qbtn);
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
    void generateSortedNames(const ClassNode *cn, const QVector<RelatedClass> &rc);
    void generateSortedQmlNames(const Node *base, const NodeList &subs);
    bool generateStatus(const Node *node);
    bool generateThreadSafeness(const Node *node);
    bool generateSince(const Node *node);
    void generateAddendum(const Node *node, Generator::Addendum type, CodeMarker *marker = nullptr,
                          bool generateNote = true) override;
    using Generator::generateBody;
    void generateBody(const Node *node);

    bool generateText(const Text &text, const Node *relative,
                      CodeMarker *marker = nullptr) override;
    const Atom *generateAtomList(const Atom *atom, const Node *relative, bool generate,
                                 int &numAtoms);
    int generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker = nullptr) override;

private:
    QXmlStreamWriter *startDocument(const Node *node);
    QXmlStreamWriter *startDocument(const ExampleNode *en, const QString &file);
    QXmlStreamWriter *startGenericDocument(const Node *node, const QString &fileName);
    void endDocument();

    void generateAnnotatedList(const Node *relative, const NodeList &nodeList,
                               const QString &selector);
    void generateAnnotatedList(const Node *relative, const NodeMultiMap &nmm,
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
    void generateAlsoList(const Node *node, CodeMarker *marker = nullptr) override;
    void generateSignatureList(const NodeList &nodes);
    void generateMaintainerList(const Aggregate *node, CodeMarker *marker = nullptr) override;
    void generateReimplementsClause(const FunctionNode *fn);
    void generateClassHierarchy(const Node *relative, NodeMap &classMap);
    void generateFunctionIndex(const Node *relative);
    void generateLegaleseList(const Node *relative);
    void generateExampleFilePage(const Node *en, const QString &file,
                                 CodeMarker *marker = nullptr) override;
    void generateOverloadedSignal(const Node *node);
    bool generateQmlText(const Text &text, const Node *relative, CodeMarker *marker = nullptr,
                         const QString &qmlName = QString()) override;
    void generateRequiredLinks(const Node *node);
    void generateLinkToExample(const ExampleNode *en, const QString &baseUrl);

    void typified(const QString &string, const Node *relative, bool trailingSpace = false,
                  bool generateType = true);
    void generateLink(const Atom *atom);
    void beginLink(const QString &link, const Node *node, const Node *relative);
    void endLink();
    inline void newLine();
    void startSectionBegin();
    void startSectionBegin(const QString &id);
    void startSectionEnd();
    void startSection(const QString &id, const QString &title);
    void endSection();
    void writeAnchor(const QString &id);
    void generateSimpleLink(const QString &href, const QString &text);
    void generateStartRequisite(const QString &description);
    void generateEndRequisite();
    void generateRequisite(const QString &description, const QString &value);
    void generateSynopsisInfo(const QString &key, const QString &value);
    void generateModifier(const QString &value);

    bool inListItemLineOpen {};
    bool inLink {};
    int currentSectionLevel {};
    QStack<int> sectionLevels {};
    QString qflagsHref_;

    QString project;
    QString projectDescription;
    QString naturalLanguage;
    QString buildversion;
    QXmlStreamWriter *writer = nullptr;

    Config *config = nullptr;
};

QT_END_NAMESPACE

#endif
