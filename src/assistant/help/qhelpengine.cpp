// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpengine.h"
#include "qhelpengine_p.h"
#include "qhelpdbreader_p.h"
#include "qhelpcontentwidget.h"
#include "qhelpindexwidget.h"
#include "qhelpsearchengine.h"
#include "qhelpcollectionhandler_p.h"
#include "qhelpfilterengine.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QPluginLoader>
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include <QtSql/QSqlQuery>

QT_BEGIN_NAMESPACE

void QHelpEnginePrivate::init(const QString &collectionFile,
                              QHelpEngineCore *helpEngineCore)
{
    QHelpEngineCorePrivate::init(collectionFile, helpEngineCore);

    if (!contentModel)
        contentModel = new QHelpContentModel(this);
    if (!indexModel)
        indexModel = new QHelpIndexModel(this);

    connect(helpEngineCore, &QHelpEngineCore::setupFinished,
            this, &QHelpEnginePrivate::scheduleApplyCurrentFilter);
    connect(helpEngineCore, &QHelpEngineCore::currentFilterChanged,
            this, &QHelpEnginePrivate::scheduleApplyCurrentFilter);
    connect(helpEngineCore->filterEngine(), &QHelpFilterEngine::filterActivated,
            this, &QHelpEnginePrivate::scheduleApplyCurrentFilter);
}

void QHelpEnginePrivate::scheduleApplyCurrentFilter()
{
    if (!error.isEmpty())
        return;

    if (m_isApplyCurrentFilterScheduled)
        return;

    m_isApplyCurrentFilterScheduled = true;
    QTimer::singleShot(0, this, &QHelpEnginePrivate::applyCurrentFilter);
}

void QHelpEnginePrivate::applyCurrentFilter()
{
    m_isApplyCurrentFilterScheduled = false;
    const QString filter = usesFilterEngine
            ? q->filterEngine()->activeFilter()
            : currentFilter;
    contentModel->createContents(filter);
    indexModel->createIndex(filter);
}

void QHelpEnginePrivate::setContentsWidgetBusy()
{
#if QT_CONFIG(cursor)
    contentWidget->setCursor(Qt::WaitCursor);
#endif
}

void QHelpEnginePrivate::unsetContentsWidgetBusy()
{
#if QT_CONFIG(cursor)
    contentWidget->unsetCursor();
#endif
}

void QHelpEnginePrivate::setIndexWidgetBusy()
{
#if QT_CONFIG(cursor)
    indexWidget->setCursor(Qt::WaitCursor);
#endif
}

void QHelpEnginePrivate::unsetIndexWidgetBusy()
{
#if QT_CONFIG(cursor)
    indexWidget->unsetCursor();
#endif
}

/*!
    \class QHelpEngine
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpEngine class provides access to contents and
    indices of the help engine.
*/

/*!
    Constructs a new help engine with the given \a parent. The help
    engine uses the information stored in the \a collectionFile for
    providing help. If the collection file does not already exist,
    it will be created.
*/
QHelpEngine::QHelpEngine(const QString &collectionFile, QObject *parent)
    : QHelpEngineCore(d = new QHelpEnginePrivate(), parent)
{
    d->init(collectionFile, this);
}

/*!
    Destroys the help engine object.
*/
QHelpEngine::~QHelpEngine()
{
}

/*!
    Returns the content model.
*/
QHelpContentModel *QHelpEngine::contentModel() const
{
    return d->contentModel;
}

/*!
    Returns the index model.
*/
QHelpIndexModel *QHelpEngine::indexModel() const
{
    return d->indexModel;
}

/*!
    Returns the content widget.
*/
QHelpContentWidget *QHelpEngine::contentWidget()
{
    if (!d->contentWidget) {
        d->contentWidget = new QHelpContentWidget();
        d->contentWidget->setModel(d->contentModel);
        connect(d->contentModel, &QHelpContentModel::contentsCreationStarted,
                d, &QHelpEnginePrivate::setContentsWidgetBusy);
        connect(d->contentModel, &QHelpContentModel::contentsCreated,
                d, &QHelpEnginePrivate::unsetContentsWidgetBusy);
    }
    return d->contentWidget;
}

/*!
    Returns the index widget.
*/
QHelpIndexWidget *QHelpEngine::indexWidget()
{
    if (!d->indexWidget) {
        d->indexWidget = new QHelpIndexWidget();
        d->indexWidget->setModel(d->indexModel);
        connect(d->indexModel, &QHelpIndexModel::indexCreationStarted,
                d, &QHelpEnginePrivate::setIndexWidgetBusy);
        connect(d->indexModel, &QHelpIndexModel::indexCreated,
                d, &QHelpEnginePrivate::unsetIndexWidgetBusy);
    }
    return d->indexWidget;
}

/*!
    Returns the default search engine.
*/
QHelpSearchEngine* QHelpEngine::searchEngine()
{
    if (!d->searchEngine)
        d->searchEngine = new QHelpSearchEngine(this, this);
    return d->searchEngine;
}

QT_END_NAMESPACE
