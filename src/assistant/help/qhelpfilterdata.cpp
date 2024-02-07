// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpfilterdata.h"

#include <QtCore/qversionnumber.h>

QT_BEGIN_NAMESPACE

class QHelpFilterDataPrivate : public QSharedData
{
public:
    QHelpFilterDataPrivate() = default;
    QHelpFilterDataPrivate(const QHelpFilterDataPrivate &other)
        : QSharedData(other)
        , m_components(other.m_components)
        , m_versions(other.m_versions)
    {}

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
QHelpFilterData::QHelpFilterData() : d(new QHelpFilterDataPrivate) { }

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
    return (d->m_components == other.d->m_components && d->m_versions == other.d->m_versions);
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
