/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qhelpfilterdata.h"
#include <QtCore/QVersionNumber>

QT_BEGIN_NAMESPACE

class QHelpFilterDataPrivate : public QSharedData
{
public:
    QHelpFilterDataPrivate() = default;
    QHelpFilterDataPrivate(const QHelpFilterDataPrivate &other)
        : QSharedData(other)
        , m_components(other.m_components)
        , m_versions(other.m_versions)
    { }
    ~QHelpFilterDataPrivate() = default;

    QStringList m_components;
    QList<QVersionNumber> m_versions;
};

/*!
    \class QHelpFilterData
    \since 5.13
    \inmodule QtHelp
    \brief The QHelpFilterData class provides details for the filters
    used by QHelpFilterEngine.

    By using setComponents() you may constrain the search results to
    documents that belong only to components specified on the given list.
    By using setVersions() you may constrain the search results to
    documents that belong only to versions specified on the given list.

    \sa QHelpFilterEngine
*/

/*!
    Constructs the empty filter.
*/
QHelpFilterData::QHelpFilterData()
    : d(new QHelpFilterDataPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QHelpFilterData::QHelpFilterData(const QHelpFilterData &) = default;

/*!
    Move-constructs a QHelpFilterData instance, making it point at the same object that \a other was pointing to.
*/
QHelpFilterData::QHelpFilterData(QHelpFilterData &&) = default;

/*!
    Destroys the filter.
*/
QHelpFilterData::~QHelpFilterData() = default;

/*!
    Assigns \a other to this filter and returns a reference to this filter.
*/
QHelpFilterData &QHelpFilterData::operator=(const QHelpFilterData &) = default;


/*!
    Move-assigns \a other to this QHelpFilterData instance.
*/
QHelpFilterData &QHelpFilterData::operator=(QHelpFilterData &&) = default;

/*!
    \fn void QHelpFilterData::swap(QHelpFilterData &other)

    Swaps the filter \a other with this filter. This
    operation is very fast and never fails.
*/

bool QHelpFilterData::operator==(const QHelpFilterData &other) const
{
    return (d->m_components == other.d->m_components &&
            d->m_versions == other.d->m_versions);
}

/*!
    Specifies the component list that is used for filtering
    the search results. Only results from components in the list
    \a components shall be returned.
*/
void QHelpFilterData::setComponents(const QStringList &components)
{
    d->m_components = components;
}

/*!
    Specifies the version list that is used for filtering
    the search results. Only results from versions in the list
    \a versions shall be returned.
*/
void QHelpFilterData::setVersions(const QList<QVersionNumber> &versions)
{
    d->m_versions = versions;
}

/*!
    Returns the component list that is used for filtering
    the search results.
*/
QStringList QHelpFilterData::components() const
{
    return d->m_components;
}

/*!
    Returns the version list that is used for filtering
    the search results.
*/
QList<QVersionNumber> QHelpFilterData::versions() const
{
    return d->m_versions;
}

QT_END_NAMESPACE
