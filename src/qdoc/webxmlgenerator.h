/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
    WebXMLGenerator() = default;

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
    void generateExampleFilePage(const Node *en, const QString &file, CodeMarker *marker) override;
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
