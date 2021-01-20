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

#ifndef HELPPROJECTWRITER_H
#define HELPPROJECTWRITER_H

#include "node.h"

#include <QtCore/qstring.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class QDocDatabase;
class Generator;
typedef QPair<QString, const Node *> QStringNodePair;

using NodeTypeSet = QSet<unsigned char>;

struct SubProject
{
    QString title;
    QString indexTitle;
    NodeTypeSet selectors;
    bool sortPages;
    QString type;
    QHash<QString, const Node *> nodes;
    QStringList groups;
};

/*
 * Name is the human-readable name to be shown in Assistant.
 * Ids is a list of unique identifiers.
 * Ref is the location of the documentation for the keyword.
 */
struct Keyword {
    QString name;
    QStringList ids;
    QString ref;
    Keyword(QString name, QString id, QString ref) : name(name), ids(QStringList(id)), ref(ref) {}
    Keyword(QString name, QStringList ids, QString ref) : name(name), ids(ids), ref(ref) {}
    bool operator<(const Keyword &o) const
    {
        // Order by name; use ref as a secondary sort key
        return (name == o.name) ? ref < o.ref : name < o.name;
    }
};

struct HelpProject
{
    using NodeStatusSet = QSet<unsigned char>;

    QString name;
    QString helpNamespace;
    QString virtualFolder;
    QString version;
    QString fileName;
    QString indexRoot;
    QString indexTitle;
    QList<Keyword> keywords;
    QSet<QString> files;
    QSet<QString> extraFiles;
    QSet<QString> filterAttributes;
    QHash<QString, QSet<QString>> customFilters;
    QSet<QString> excluded;
    QList<SubProject> subprojects;
    QHash<const Node *, NodeStatusSet> memberStatus;
    bool includeIndexNodes;
};


class HelpProjectWriter
{
public:
    HelpProjectWriter(const QString &defaultFileName, Generator *g);
    void reset(const QString &defaultFileName, Generator *g);
    void addExtraFile(const QString &file);
    void addExtraFiles(const QSet<QString> &files);
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

    QDocDatabase *qdb_;
    Generator *gen_;

    QString outputDir;
    QList<HelpProject> projects;
};

QT_END_NAMESPACE

#endif
