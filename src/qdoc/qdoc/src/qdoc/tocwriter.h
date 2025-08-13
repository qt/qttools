// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TOCWRITER
#define TOCWRITER

#include "node.h"

#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

#include <optional>
#include <utility>

QT_BEGIN_NAMESPACE

class Generator;
class QDocDatabase;
class QXmlStreamWriter;

class TOCWriter
{
public:
    using TitledNodeList = QList<std::pair<const Node*, QString>>;

    TOCWriter(Generator *g, const QString &project);
    TOCWriter(const TOCWriter &) = delete;
    TOCWriter &operator=(const TOCWriter &) = delete;

    void generateTOC(const QString &fileName, const QString &indexTitle);

private:
    void writeEntry(QXmlStreamWriter &writer, const Node *node);
    [[nodiscard]] std::optional<TitledNodeList> getEntries(const Node *node) const;

private:
    Generator *m_gen;
    QDocDatabase *m_qdb;
    int m_recursionDepth{0};
    QString m_project{};
};

QT_END_NAMESPACE

#endif // TOCWRITER

