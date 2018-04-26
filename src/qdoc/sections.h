/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef SECTIONS_H
#define SECTIONS_H

#include <qpair.h>
#include "node.h"

QT_BEGIN_NAMESPACE

typedef QMultiMap<QString, Node*> MemberMap; // the string is the member signature
typedef QPair<const QmlTypeNode*, MemberMap> ClassMap;    // the node is the QML type
typedef QList<ClassMap*> ClassMapList;

typedef QPair<QStringList, NodeList> KeysAndNodes;
typedef QPair<const QmlTypeNode*, KeysAndNodes> ClassKeysNodes;
typedef QList<ClassKeysNodes*> ClassKeysNodesList;

struct Section
{
    QString name_;
    QString divClass_;
    QString singular_;
    QString plural_;
    QStringList keys_;
    NodeList members_;
    NodeList reimpMembers_;
    QList<QPair<Aggregate *, int> > inherited_;
    ClassKeysNodesList classKeysNodesList_;

    Section() { }
    Section(const QString& name,
            const QString& divClass,
            const QString& singular,
            const QString& plural)
        : name_(name), divClass_(divClass), singular_(singular), plural_(plural) { }
    ~Section();

    void appendMember(Node* node) { members_.append(node); }
    void appendReimpMember(Node* node) { reimpMembers_.append(node); }
};

struct FastSection
{
    QString name_;
    QString divClass_;
    QString singular_;
    QString plural_;
    QMultiMap<QString, Node *> memberMap_;
    QMultiMap<QString, Node *> reimpMemberMap_;
    ClassMapList classMapList_;
    QList<QPair<Aggregate *, int> > inherited_;

    FastSection(const QString& name, const QString& singular, const QString& plural)
        : name_(name), singular_(singular), plural_(plural) { }

    FastSection(const QString& name,
                const QString& divClass,
                const QString& singular,
                const QString& plural)
        : name_(name), divClass_(divClass), singular_(singular), plural_(plural) { }

    ~FastSection();

    void clear();
    bool isEmpty() const {
        return (memberMap_.isEmpty() &&
                inherited_.isEmpty() &&
                reimpMemberMap_.isEmpty() &&
                classMapList_.isEmpty());
    }

};

class Sections
{
 public:
    enum Style { Summary, Detailed, Subpage, Accessors };
    enum Status { Obsolete, Okay };

    Sections() { }
    ~Sections() { }

    static QList<Section> getStdCppSections(const Aggregate *aggregate, Style style, Status status);
    static QList<Section> getStdQmlSections(Aggregate* aggregate, Style style, Status status = Okay);
    static QString sortName(const Node *node, const QString* name = 0);
    static void insert(FastSection &fs, Node *node, Style style, Status status);
    static bool insertReimpFunc(FastSection& fs, Node* node, Status status);
    static void append(QList<Section>& sections, const FastSection& fs, bool includeKeys = false);
    static void setCurrentNode(const Aggregate *t) { aggregate_ = t; }

 private:
    static void getAllCppClassMembers(QList<Section> &sections, Style style, Status status);
    static void getCppClassStdSummarySections(QList<Section> &sections, Style style, Status status);
    static void getCppClassStdDetailedSections(QList<Section> &sections, Style style, Status status);
    static void getAllStdCppSections(QList<Section> &sections, Style style, Status status);
    static void getAllQmlTypeMembers(QList<Section> &sections);
    static void getQmlTypeStdSummarySections(QList<Section> &sections, Style style, Status status);
    static void getQmlTypeStdDetailedSections(QList<Section> &sections, Style style, Status status);

 private:
    static const Aggregate   *aggregate_;
};

QT_END_NAMESPACE

#endif
