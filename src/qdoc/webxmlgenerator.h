// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef WEBXMLGENERATOR_H
#define WEBXMLGENERATOR_H

#include "codemarker.h"
#include "htmlgenerator.h"
#include "qdocindexfiles.h"

#include <QtCore/qscopedpointer.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class Aggregate;

class WebXMLGenerator : public HtmlGenerator, public IndexSectionWriter
{
public:
    WebXMLGenerator(FileResolver& file_resolver);

    void initializeGenerator() override;
    void terminateGenerator() override;
    QString format() override;
    // from IndexSectionWriter
    void append(QXmlStreamWriter &writer, Node *node) override;

protected:
    qsizetype generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker) override;
    void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) override;
    void generatePageNode(PageNode *pn, CodeMarker *marker) override;
    void generateDocumentation(Node *node) override;
    void generateExampleFilePage(const Node *en, ResolvedFile file, CodeMarker *marker = nullptr) override;
    [[nodiscard]] QString fileExtension() const override;

    virtual const Atom *addAtomElements(QXmlStreamWriter &writer, const Atom *atom,
                                        const Node *relative, CodeMarker *marker);
    virtual void generateIndexSections(QXmlStreamWriter &writer, Node *node);

private:
    void generateAnnotatedList(QXmlStreamWriter &writer, const Node *relative,
                               const NodeMap &nodeMap);
    void generateAnnotatedList(QXmlStreamWriter &writer, const Node *relative,
                               const NodeList &nodeList);
    void generateRelations(QXmlStreamWriter &writer, const Node *node);
    void startLink(QXmlStreamWriter &writer, const Atom *atom, const Node *node,
                   const QString &link);
    void endLink(QXmlStreamWriter &writer);
    QString fileBase(const Node *node) const override;

    bool m_hasQuotingInformation { false };
    QString quoteCommand {};
    QScopedPointer<QXmlStreamWriter> currentWriter {};
    bool m_supplement { false };
};

QT_END_NAMESPACE

#endif
