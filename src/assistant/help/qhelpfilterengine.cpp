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

#include "qhelpfilterengine.h"
#include "qhelpenginecore.h"
#include "qhelpfilterdata.h"
#include "qhelpdbreader_p.h"
#include "qhelpcollectionhandler_p.h"

#include <QtCore/QThread>
#include <QtCore/QVersionNumber>

QT_BEGIN_NAMESPACE

static const char ActiveFilter[] = "activeFilter";

class QHelpFilterEnginePrivate
{
public:
    bool setup();

    QHelpFilterEngine *q = nullptr;
    QHelpEngineCore *m_helpEngine = nullptr;
    QHelpCollectionHandler *m_collectionHandler = nullptr;
    QString m_currentFilter;
    bool m_needsSetup = true;
};

bool QHelpFilterEnginePrivate::setup()
{
    if (!m_collectionHandler)
        return false;

    if (!m_needsSetup)
        return true;

    if (!m_helpEngine->setupData())
        return false;

    m_needsSetup = false;

    const QString filter = m_collectionHandler->customValue(
                QLatin1String(ActiveFilter), QString()).toString();
    if (!filter.isEmpty() && m_collectionHandler->filters().contains(filter))
        m_currentFilter = filter;

    emit q->filterActivated(m_currentFilter);
    return true;
}

//////////////

/*!
    \class QHelpFilterEngine
    \since 5.13
    \inmodule QtHelp
    \brief The QHelpFilterEngine class provides a filtered view of the
    help contents.

    The filter engine allows the management of filters associated with
    a QHelpEngineCore instance. The help engine internally creates an
    instance of the filter engine, which can be accessed by calling
    QHelpEngineCore::filterEngine(). Therefore, the public constructor
    of this class is disabled.

    The filters are identified by a filter name string. Filter details are
    described by the \l QHelpFilterData class.

    The filter engine allows for adding new filters and changing the existing
    filters' data through the setFilterData() method. An existing filter can
    be removed through the removeFilter() method.

    Out of the registered filters one can be marked as the active one.
    The active filter will be used by the associated help engine for returning
    filtered results of many different functions, such as content, index, or
    search results. If no filter is marked active, the help engine returns the
    full results list available.

    The active filter is returned by activeFilter() and it can be changed by
    setActiveFilter().

    \sa QHelpEngineCore
*/

/*!
    \fn void QHelpFilterEngine::filterActivated(const QString &newFilter)

    This signal is emitted when the active filter is set. \a newFilter
    specifies the name of the filter.

    \sa setActiveFilter()
*/

/*!
    \internal
    Constructs the filter engine for \a helpEngine.
*/
QHelpFilterEngine::QHelpFilterEngine(QHelpEngineCore *helpEngine)
    : QObject(helpEngine),
      d(new QHelpFilterEnginePrivate)
{
    d->q = this;
    d->m_helpEngine = helpEngine;
}

/*!
    \internal
    Destroys the existing filter engine.
*/
QHelpFilterEngine::~QHelpFilterEngine()
{
    delete d;
}

/*!
    \internal
    Sets the \a collectionHandler to be used for this filter engine.
*/
void QHelpFilterEngine::setCollectionHandler(QHelpCollectionHandler *collectionHandler)
{
    d->m_collectionHandler = collectionHandler;
    d->m_currentFilter = QString();
    d->m_needsSetup = true;
}

/*!
    Returns the map of all the available namespaces as keys
    together with their associated components as values.
*/
QMap<QString, QString> QHelpFilterEngine::namespaceToComponent() const
{
    if (!d->setup())
        return QMap<QString, QString>();
    return d->m_collectionHandler->namespaceToComponent();
}

/*!
    Returns the map of all the available namespaces as keys
    together with their associated versions as values.
*/
QMap<QString, QVersionNumber> QHelpFilterEngine::namespaceToVersion() const
{
    if (!d->setup())
        return QMap<QString, QVersionNumber>();

    return d->m_collectionHandler->namespaceToVersion();
}

/*!
    Returns the list of all filter names defined inside the filter engine.
*/
QStringList QHelpFilterEngine::filters() const
{
    if (!d->setup())
        return QStringList();
    return d->m_collectionHandler->filters();
}

/*!
    Returns the list of all available components defined in all
    registered documentation files.
*/
QStringList QHelpFilterEngine::availableComponents() const
{
    if (!d->setup())
        return QStringList();
    return d->m_collectionHandler->availableComponents();
}

/*!
    \since 5.15

    Returns the list of all available versions defined in all
    registered documentation files.
*/
QList<QVersionNumber> QHelpFilterEngine::availableVersions() const
{
    if (!d->setup())
        return QList<QVersionNumber>();
    return d->m_collectionHandler->availableVersions();
}

/*!
    Returns the filter details associated with \a filterName.
*/
QHelpFilterData QHelpFilterEngine::filterData(const QString &filterName) const
{
    if (!d->setup())
        return QHelpFilterData();
    return d->m_collectionHandler->filterData(filterName);
}

/*!
    Changes the existing filter details of the filter identified by
    \a filterName to \a filterData. If the filter does not exist, a
    new filter is created.

    Returns \c true if setting the filter succeeded, otherwise returns \c false.
*/
bool QHelpFilterEngine::setFilterData(const QString &filterName, const QHelpFilterData &filterData)
{
    if (!d->setup())
        return false;
    return d->m_collectionHandler->setFilterData(filterName, filterData);
}

/*!
    Removes the filter identified by \a filterName.

    Returns \c true if removing the filter succeeded, otherwise returns
    \c false.
*/
bool QHelpFilterEngine::removeFilter(const QString &filterName)
{
    if (!d->setup())
        return false;
    return d->m_collectionHandler->removeFilter(filterName);
}

/*!
    Returns the name of the currently active filter.
*/
QString QHelpFilterEngine::activeFilter() const
{
    if (!d->setup())
        return QString();
    return d->m_currentFilter;
}

/*!
    Changes the currently active filter to \a filterName.

    Returns \c true if changing the filter succeeded, otherwise
    returns \c false.
*/
bool QHelpFilterEngine::setActiveFilter(const QString &filterName)
{
    if (!d->setup())
        return false;

    if (filterName == d->m_currentFilter)
        return true;

    if (!filterName.isEmpty() && !d->m_collectionHandler->filters().contains(filterName))
        return false;

    d->m_currentFilter = filterName;
    d->m_collectionHandler->setCustomValue(QLatin1String(ActiveFilter),
            d->m_currentFilter);

    emit filterActivated(d->m_currentFilter);

    return true;
}

/*!
    Returns the list of all registered documentation namespaces that match
    the filter identified by \a filterName.
*/
QStringList QHelpFilterEngine::namespacesForFilter(const QString &filterName) const
{
    if (!d->setup())
        return QStringList();
    return  d->m_collectionHandler->namespacesForFilter(filterName);
}

/*!
    \since 5.15

    Returns a sorted list of available indices.
    The returned list contents depend on the active filter, and therefore only
    the indices registered for the active filter will be returned.
*/
QStringList QHelpFilterEngine::indices() const
{
    return indices(activeFilter());
}

/*!
    \since 5.15

    Returns a sorted list of available indices, filtered by \a filterName.
    The returned list contents depend on the passed filter, and therefore only
    the indices registered for this filter will be returned.
    If you want to get all available indices unfiltered,
    pass empty string as \a filterName.
*/
QStringList QHelpFilterEngine::indices(const QString &filterName) const
{
    if (!d->setup())
        return QStringList();
    return d->m_collectionHandler->indicesForFilter(filterName);
}

QT_END_NAMESPACE
