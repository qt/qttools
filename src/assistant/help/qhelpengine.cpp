// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpengine.h"
#include "qhelpcontentwidget.h"
#include "qhelpfilterengine.h"
#include "qhelpindexwidget.h"
#include "qhelpsearchengine.h"

#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

class QHelpEnginePrivate
{
public:
    QHelpEnginePrivate(QHelpEngineCore *helpEngineCore);

    QHelpContentModel *contentModel = nullptr;
    QHelpContentWidget *contentWidget = nullptr;

    QHelpIndexModel *indexModel = nullptr;
    QHelpIndexWidget *indexWidget = nullptr;

    QHelpSearchEngine *searchEngine = nullptr;

    bool m_isApplyCurrentFilterScheduled = false;
    QHelpEngineCore *m_helpEngineCore = nullptr;
};

QHelpEnginePrivate::QHelpEnginePrivate(QHelpEngineCore *helpEngineCore)
    : m_helpEngineCore(helpEngineCore)
{
    if (!contentModel)
        contentModel = new QHelpContentModel(helpEngineCore);
    if (!indexModel)
        indexModel = new QHelpIndexModel(m_helpEngineCore);

    const auto applyCurrentFilter = [this] {
        m_isApplyCurrentFilterScheduled = false;
        contentModel->createContentsForCurrentFilter();
        indexModel->createIndexForCurrentFilter();
    };

    const auto scheduleApplyCurrentFilter = [this, applyCurrentFilter] {
        if (!m_helpEngineCore->error().isEmpty())
            return;

        if (m_isApplyCurrentFilterScheduled)
            return;

        m_isApplyCurrentFilterScheduled = true;
        QTimer::singleShot(0, m_helpEngineCore, applyCurrentFilter);
    };

    QObject::connect(m_helpEngineCore, &QHelpEngineCore::setupFinished,
                     m_helpEngineCore, scheduleApplyCurrentFilter);
    QObject::connect(m_helpEngineCore, &QHelpEngineCore::currentFilterChanged,
                     m_helpEngineCore, scheduleApplyCurrentFilter);
    QObject::connect(m_helpEngineCore->filterEngine(), &QHelpFilterEngine::filterActivated,
                     m_helpEngineCore, scheduleApplyCurrentFilter);
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
    : QHelpEngineCore(collectionFile, parent)
    , d(new QHelpEnginePrivate(this))
{}

/*!
    Destroys the help engine object.
*/
QHelpEngine::~QHelpEngine()
{
    delete d;
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
        d->contentWidget = new QHelpContentWidget;
        d->contentWidget->setModel(d->contentModel);
#if QT_CONFIG(cursor)
        connect(d->contentModel, &QHelpContentModel::contentsCreationStarted, this, [this] {
            d->contentWidget->setCursor(Qt::WaitCursor);
        });
        connect(d->contentModel, &QHelpContentModel::contentsCreated, this, [this] {
            d->contentWidget->unsetCursor();
        });
#endif
    }
    return d->contentWidget;
}

/*!
    Returns the index widget.
*/
QHelpIndexWidget *QHelpEngine::indexWidget()
{
    if (!d->indexWidget) {
        d->indexWidget = new QHelpIndexWidget;
        d->indexWidget->setModel(d->indexModel);
#if QT_CONFIG(cursor)
        connect(d->indexModel, &QHelpIndexModel::indexCreationStarted, this, [this] {
            d->indexWidget->setCursor(Qt::WaitCursor);
        });
        connect(d->indexModel, &QHelpIndexModel::indexCreated, this, [this] {
            d->indexWidget->unsetCursor();
        });
#endif
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
