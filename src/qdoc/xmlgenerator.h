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

#ifndef XMLGENERATOR_H
#define XMLGENERATOR_H

#include "node.h"
#include "generator.h"

#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class XmlGenerator : public Generator
{
public:
    explicit XmlGenerator() = default;

protected:
    QHash<QString, QString> refMap;

    static bool hasBrief(const Node *node);
    static bool isThreeColumnEnumValueTable(const Atom *atom);
    static int hOffset(const Node *node);

    static void rewritePropertyBrief(const Atom *atom, const Node *relative);
    static Node::NodeType typeFromString(const Atom *atom);
    static void setImageFileName(const Node *relative, const QString &fileName);
    static QPair<QString, int> getAtomListValue(const Atom *atom);
    static QPair<QString, QString> getTableWidthAttr(const Atom *atom);

    QString registerRef(const QString &ref);
    QString refForNode(const Node *node);
    QString linkForNode(const Node *node, const Node *relative);
    QString getLink(const Atom *atom, const Node *relative, const Node **node);
    QString getAutoLink(const Atom *atom, const Node *relative, const Node **node);

    const QPair<QString, QString> anchorForNode(const Node *node);

    static QString targetType(const Node *node);
};

QT_END_NAMESPACE

#endif // XMLGENERATOR_H
