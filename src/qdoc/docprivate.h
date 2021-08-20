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
#ifndef DOCPRIVATE_H
#define DOCPRIVATE_H

#include "atom.h"
#include "config.h"
#include "codemarker.h"
#include "doc.h"
#include "editdistance.h"
#include "generator.h"
#include "utilities.h"
#include "openedlist.h"
#include "quoter.h"
#include "text.h"
#include "tokenizer.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qhash.h>
#include <QtCore/qmap.h>
#include <QtCore/qtextstream.h>

#include <cctype>
#include <climits>
#include <utility>

QT_BEGIN_NAMESPACE

typedef QMap<QString, ArgList> CommandMap;

struct DocPrivateExtra
{
    QList<Atom *> m_tableOfContents {};
    QList<int> m_tableOfContentsLevels {};
    QList<Atom *> m_keywords {};
    QList<Atom *> m_targets {};
    QStringMultiMap m_metaMap {};
};

class DocPrivate
{
public:
    explicit DocPrivate(const Location &start = Location(), const Location &end = Location(),
                        QString source = QString())
        : m_start_loc(start), m_end_loc(end), m_src(std::move(source)), m_hasLegalese(false) {};
    ~DocPrivate();

    void addAlso(const Text &also);
    void constructExtra();
    void ref() { ++count; }
    bool deref() { return (--count == 0); }

    int count { 1 };
    // ### move some of this in DocPrivateExtra
    Location m_start_loc {};
    Location m_end_loc {};
    QString m_src {};
    Text m_text {};
    QSet<QString> m_params {};
    QList<Text> m_alsoList {};
    QStringList m_enumItemList {};
    QStringList m_omitEnumItemList {};
    QSet<QString> m_metacommandsUsed {};
    CommandMap m_metaCommandMap {};
    DocPrivateExtra *extra { nullptr };
    TopicList m_topics {};

    bool m_hasLegalese : 1;
};

QT_END_NAMESPACE

#endif // DOCPRIVATE_H
