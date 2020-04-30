/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
#ifndef DITAREF_H
#define DITAREF_H

#include "location.h"

#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>

class DitaRef
{
public:
    DitaRef() = default;
    virtual ~DitaRef() = default;

    const QString &navtitle() const { return m_navTitle; }
    const QString &href() const { return m_href; }
    void setNavtitle(const QString &title) { m_navTitle = title; }
    void setHref(const QString &t) { m_href = t; }
    virtual bool isMapRef() const = 0;
    virtual const QVector<DitaRef *> *subrefs() const { return nullptr; }
    virtual void appendSubref(DitaRef *) {}

private:
    QString m_navTitle;
    QString m_href;
};

typedef QVector<DitaRef *> DitaRefList;

class TopicRef : public DitaRef
{
public:
    TopicRef() = default;
    ~TopicRef() override { qDeleteAll(m_subRefs); };
    bool isMapRef() const override { return false; }
    const QVector<DitaRef *> *subrefs() const override { return &m_subRefs; }
    void appendSubref(DitaRef *t) override { m_subRefs.append(t); }

private:
    DitaRefList m_subRefs;
};

class MapRef : public DitaRef
{
public:
    MapRef() {}

    bool isMapRef() const override { return true; }
};

#endif // DITAREF_H
