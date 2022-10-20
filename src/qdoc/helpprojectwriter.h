/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef HELPPROJECTWRITER_H
#define HELPPROJECTWRITER_H

#include "node.h"

#include <QtCore/qstring.h>
#include <QtCore/qxmlstream.h>

#include <utility>

QT_BEGIN_NAMESPACE

class QDocDatabase;
class Generator;

using NodeTypeSet = QSet<unsigned char>;

struct SubProject
{
    QString m_title {};
    QString m_indexTitle {};
    NodeTypeSet m_selectors {};
    bool m_sortPages {};
    QString m_type {};
    QHash<QString, const Node *> m_nodes {};
    QStringList m_groups {};
};

/*
 * Name is the human-readable name to be shown in Assistant.
 * Ids is a list of unique identifiers.
 * Ref is the location of the documentation for the keyword.
 */
struct Keyword {
    QString m_name {};
    QStringList m_ids {};
    QString m_ref {};
    Keyword(QString name, const QString &id, QString ref)
        : m_name(std::move(name)), m_ids(QStringList(id)), m_ref(std::move(ref))
    {
    }
    Keyword(QString name, QStringList ids, QString ref)
        : m_name(std::move(name)), m_ids(std::move(ids)), m_ref(std::move(ref))
    {
    }
    bool operator<(const Keyword &o) const
    {
        // Order by name; use ref as a secondary sort key
        return (m_name == o.m_name) ? m_ref < o.m_ref : m_name < o.m_name;
    }
};

struct HelpProject
{
    using NodeStatusSet = QSet<unsigned char>;

    QString m_name {};
    QString m_helpNamespace {};
    QString m_virtualFolder {};
    QString m_version {};
    QString m_fileName {};
    QString m_indexRoot {};
    QString m_indexTitle {};
    QList<Keyword> m_keywords {};
    QSet<QString> m_files {};
    QSet<QString> m_extraFiles {};
    QSet<QString> m_filterAttributes {};
    QHash<QString, QSet<QString>> m_customFilters {};
    QSet<QString> m_excluded {};
    QList<SubProject> m_subprojects {};
    QHash<const Node *, NodeStatusSet> m_memberStatus {};
    bool m_includeIndexNodes {};
};


class HelpProjectWriter
{
public:
    HelpProjectWriter(const QString &defaultFileName, Generator *g);
    void reset(const QString &defaultFileName, Generator *g);
    void addExtraFile(const QString &file);
    void generate();

private:
    void generateProject(HelpProject &project);
    void generateSections(HelpProject &project, QXmlStreamWriter &writer, const Node *node);
    bool generateSection(HelpProject &project, QXmlStreamWriter &writer, const Node *node);
    Keyword keywordDetails(const Node *node) const;
    void writeHashFile(QFile &file);
    void writeNode(HelpProject &project, QXmlStreamWriter &writer, const Node *node);
    void readSelectors(SubProject &subproject, const QStringList &selectors);
    void addMembers(HelpProject &project, QXmlStreamWriter &writer, const Node *node);
    void writeSection(QXmlStreamWriter &writer, const QString &path, const QString &value);

    QDocDatabase *m_qdb {};
    Generator *m_gen {};

    QString m_outputDir {};
    QList<HelpProject> m_projects {};
};

QT_END_NAMESPACE

#endif
