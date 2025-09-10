// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpfilterengine.h"
#include "qhelpcollectionhandler_p.h"
#include "qhelpenginecore.h"
#include "qhelpfilterdata.h"

#include <QtCore/qthread.h>
#include <QtCore/qversionnumber.h>

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

    // Prevent endless loop when connected to setupFinished() signal
    // and using from there QHelpFilterEngine, causing setup() to be
    // called in turn.
    m_needsSetup = false;

    if (!m_helpEngine->setupData()) {
        m_needsSetup = true;
        return false;
    }

    const QString filter = m_collectionHandler->customValue(
                QLatin1StringView(ActiveFilter), QString()).toString();
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
    d->m_currentFilter.clear();
    d->m_needsSetup = true;
}

/*!
    Returns the map of all the available namespaces as keys
    together with their associated components as values.
*/
QMap<QString, QString> QHelpFilterEngine::namespaceToComponent() const
{
    if (!d->setup())
        return {};
    return d->m_collectionHandler->namespaceToComponent();
}

/*!
    Returns the map of all the available namespaces as keys
    together with their associated versions as values.
*/
QMap<QString, QVersionNumber> QHelpFilterEngine::namespaceToVersion() const
{
    if (!d->setup())
        return {};
    return d->m_collectionHandler->namespaceToVersion();
}

/*!
    Returns the list of all filter names defined inside the filter engine.
*/
QStringList QHelpFilterEngine::filters() const
{
    if (!d->setup())
        return {};
    return d->m_collectionHandler->filters();
}

/*!
    Returns the list of all available components defined in all
    registered documentation files.
*/
QStringList QHelpFilterEngine::availableComponents() const
{
    if (!d->setup())
        return {};
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
        return {};
    return d->m_collectionHandler->availableVersions();
}

/*!
    Returns the filter details associated with \a filterName.
*/
QHelpFilterData QHelpFilterEngine::filterData(const QString &filterName) const
{
    if (!d->setup())
        return {};
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
        return {};
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
    d->m_collectionHandler->setCustomValue(QLatin1StringView(ActiveFilter), d->m_currentFilter);
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
        return {};
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
        return {};
    return d->m_collectionHandler->indicesForFilter(filterName);
}

QT_END_NAMESPACE
