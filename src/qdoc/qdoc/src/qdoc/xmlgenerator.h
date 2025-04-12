// Copyright (C) 2019 Thibaut Cuvelier
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef XMLGENERATOR_H
#define XMLGENERATOR_H

#include "generator.h"
#include "genustypes.h"
#include "node.h"

#include "filesystem/fileresolver.h"

#include <QtCore/qmap.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class XmlGenerator : public Generator
{
public:
    explicit XmlGenerator(FileResolver& file_resolver);

protected:
    QHash<QString, QString> refMap;

    static bool hasBrief(const Node *node);
    static bool isThreeColumnEnumValueTable(const Atom *atom);
    static bool isOneColumnValueTable(const Atom *atom);
    static int hOffset(const Node *node);

    static void rewritePropertyBrief(const Atom *atom, const Node *relative);
    static NodeType typeFromString(const Atom *atom);
    static void setImageFileName(const Node *relative, const QString &fileName);
    static std::pair<QString, int> getAtomListValue(const Atom *atom);
    static std::pair<QString, QString> getTableWidthAttr(const Atom *atom);

    QString registerRef(const QString &ref, bool xmlCompliant = false);
    QString refForNode(const Node *node);
    QString linkForNode(const Node *node, const Node *relative);
    QString getLink(const Atom *atom, const Node *relative, const Node **node);
    QString getAutoLink(const Atom *atom, const Node *relative, const Node **node,
                        Genus = Genus::DontCare);

    std::pair<QString, QString> anchorForNode(const Node *node);

    static QString targetType(const Node *node);

protected:
    static const QRegularExpression m_funcLeftParen;
    const Node *m_linkNode { nullptr };
};

QT_END_NAMESPACE

#endif // XMLGENERATOR_H
