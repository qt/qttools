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

#include <qhash.h>
#include <qregexp.h>
#include <qxmlstream.h>
#include "codemarker.h"
#include "xmlgenerator.h"

#include <QtCore/qxmlstream.h>

#include "codemarker.h"

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

    void generateList(QXmlStreamWriter &writer, const Node *relative, const QString &selector);
    void generateHeader(QXmlStreamWriter &writer, const QString &title, const QString &subtitle,
                        const Node *node);
    void closeTextSections(QXmlStreamWriter &writer);
    void generateFooter(QXmlStreamWriter &writer);
    void generateDocBookSynopsis(QXmlStreamWriter &writer, const Node *node);
    void generateRequisites(QXmlStreamWriter &writer, const Aggregate *inner);
    void generateQmlRequisites(QXmlStreamWriter &writer, const QmlTypeNode *qcn);
    void generateSortedNames(QXmlStreamWriter &writer, const ClassNode *cn,
                             const QVector<RelatedClass> &rc);
    void generateSortedQmlNames(QXmlStreamWriter &writer, const Node *base, const NodeList &subs);
    bool generateStatus(QXmlStreamWriter &writer, const Node *node);
    bool generateThreadSafeness(QXmlStreamWriter &writer, const Node *node);
    bool generateSince(QXmlStreamWriter &writer, const Node *node);
    using Generator::generateBody;
    void generateBody(QXmlStreamWriter &writer, const Node *node);

    using Generator::generateText;
    bool generateText(QXmlStreamWriter &writer, const Text &text, const Node *relative);
    const Atom *generateAtomList(QXmlStreamWriter &writer, const Atom *atom, const Node *relative,
                                 bool generate, int &numAtoms);
    using Generator::generateAtom;
    int generateAtom(QXmlStreamWriter &writer, const Atom *atom, const Node *relative);

private:
    QXmlStreamWriter *startDocument(const Node *node);
    QXmlStreamWriter *startDocument(const ExampleNode *en, const QString &file);
    QXmlStreamWriter *startGenericDocument(const Node *node, const QString &fileName);
    static void endDocument(QXmlStreamWriter *writer);

    void generateAnnotatedList(QXmlStreamWriter &writer, const Node *relative,
                               const NodeList &nodeList, const QString &selector);
    void generateAnnotatedList(QXmlStreamWriter &writer, const Node *relative,
                               const NodeMultiMap &nmm, const QString &selector);
    void generateAnnotatedLists(QXmlStreamWriter &writer, const Node *relative,
                                const NodeMultiMap &nmm, const QString &selector);
    void generateCompactList(QXmlStreamWriter &writer, ListType listType, const Node *relative,
                             const NodeMultiMap &nmm, const QString &commonPrefix,
                             const QString &selector);
    using Generator::generateFileList;
    void generateFileList(QXmlStreamWriter &writer, const ExampleNode *en, bool images);
    void generateObsoleteMembers(QXmlStreamWriter &writer, const Sections &sections);
    void generateObsoleteQmlMembers(QXmlStreamWriter &writer, const Sections &sections);
    void generateSectionList(QXmlStreamWriter &writer, const Section &section, const Node *relative,
                             Section::Status status = Section::Active);
    void generateSectionInheritedList(QXmlStreamWriter &writer, const Section &section,
                                      const Node *relative);
    void generateSynopsisName(QXmlStreamWriter &writer, const Node *node, const Node *relative,
                              bool generateNameLink);
    void generateParameter(QXmlStreamWriter &writer, const Parameter &parameter,
                           const Node *relative, bool generateExtra, bool generateType);
    void generateSynopsis(QXmlStreamWriter &writer, const Node *node, const Node *relative,
                          Section::Style style);
    void generateEnumValue(QXmlStreamWriter &writer, const QString &enumValue,
                           const Node *relative);
    void generateDetailedMember(QXmlStreamWriter &writer, const Node *node,
                                const PageNode *relative);
    void generateDetailedQmlMember(QXmlStreamWriter &writer, Node *node, const Aggregate *relative);

    void generateFullName(QXmlStreamWriter &writer, const Node *node, const Node *relative);
    void generateFullName(QXmlStreamWriter &writer, const Node *apparentNode,
                          const QString &fullName, const Node *actualNode);
    void generateBrief(QXmlStreamWriter &writer, const Node *node);
    using Generator::generateAlsoList;
    void generateAlsoList(QXmlStreamWriter &writer, const Node *node);
    static void generateSignatureList(QXmlStreamWriter &writer, const NodeList &nodes);
    using Generator::generateMaintainerList;
    void generateMaintainerList(QXmlStreamWriter &writer, const Aggregate *node);
    void generateReimplementsClause(QXmlStreamWriter &writer, const FunctionNode *fn);
    void generateClassHierarchy(QXmlStreamWriter &writer, const Node *relative, NodeMap &classMap);
    void generateFunctionIndex(QXmlStreamWriter &writer, const Node *relative);
    void generateLegaleseList(QXmlStreamWriter &writer, const Node *relative);
    using Generator::generateExampleFilePage;
    void generateExampleFilePage(const Node *en, const QString &file);
    static void generateOverloadedSignal(QXmlStreamWriter &writer, const Node *node);
    static void generatePrivateSignalNote(QXmlStreamWriter &writer);
    static void generateInvokableNote(QXmlStreamWriter &writer, const Node *node);
    void generateAssociatedPropertyNotes(QXmlStreamWriter &writer, const FunctionNode *fn);
    using Generator::generateQmlText;
    bool generateQmlText(QXmlStreamWriter &writer, const Text &text, const Node *relative);
    void generateRequiredLinks(QXmlStreamWriter &writer, const Node *node);
    void generateLinkToExample(QXmlStreamWriter &writer, const ExampleNode *en,
                               const QString &baseUrl);

    void typified(QXmlStreamWriter &writer, const QString &string, const Node *relative,
                  bool trailingSpace = false, bool generateType = true);
    void generateLink(QXmlStreamWriter &writer, const Atom *atom);
    void beginLink(QXmlStreamWriter &writer, const QString &link, const Node *node,
                   const Node *relative);
    void endLink(QXmlStreamWriter &writer);

    bool inListItemLineOpen {};
    bool inLink {};
    int currentSectionLevel {};
    QStack<int> sectionLevels {};
    QString qflagsHref_;

    QString project;
    QString projectDescription;
    QString naturalLanguage;
    QString buildversion;
};

QT_END_NAMESPACE

#endif
