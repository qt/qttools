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

#ifndef WEBXMLGENERATOR_H
#define WEBXMLGENERATOR_H

#include "codemarker.h"
#include "htmlgenerator.h"
#include "qdocindexfiles.h"

#include <QtCore/qscopedpointer.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class WebXMLGenerator : public HtmlGenerator, public IndexSectionWriter
{
public:
    explicit WebXMLGenerator() {}

    void initializeGenerator() override;
    void terminateGenerator() override;
    QString format() override;
    // from IndexSectionWriter
    void append(QXmlStreamWriter &writer, Node *node) override;

protected:
    int generateAtom(const Atom *atom, const Node *relative, CodeMarker *marker) override;
    void generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker) override;
    void generatePageNode(PageNode *pn, CodeMarker *marker) override;
    void generateDocumentation(Node *node) override;
    void generateExampleFilePage(const Node *en, const QString &file, CodeMarker *marker) override;
    QString fileExtension() const override;

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

    bool inLink;
    bool inContents;
    bool inSectionHeading;
    bool hasQuotingInformation;
    int numTableRows;
    QString quoteCommand;
    QScopedPointer<QXmlStreamWriter> currentWriter;
    bool supplement = false;
};

QT_END_NAMESPACE

#endif
