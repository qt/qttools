// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpcontentwidget.h"
#include "qhelpenginecore.h"

#if QT_CONFIG(future)
#include <QtCore/qfuturewatcher.h>
#endif

#include <QtCore/qdir.h>
#include <QtWidgets/qheaderview.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class QHelpContentModelPrivate
{
#if QT_CONFIG(future)
    using ItemFutureProvider = std::function<QFuture<std::shared_ptr<QHelpContentItem>>()>;

    struct WatcherDeleter
    {
        void operator()(QFutureWatcherBase *watcher) {
            watcher->disconnect();
            watcher->cancel();
            watcher->waitForFinished();
            delete watcher;
        }
    };
#endif

public:
#if QT_CONFIG(future)
    void createContents(const ItemFutureProvider &futureProvider);
#endif

    QHelpContentModel *q = nullptr;
    QHelpEngineCore *helpEngine = nullptr;
    std::shared_ptr<QHelpContentItem> rootItem = {};
#if QT_CONFIG(future)
    std::unique_ptr<QFutureWatcher<std::shared_ptr<QHelpContentItem>>, WatcherDeleter> watcher = {};
#endif
};

#if QT_CONFIG(future)
void QHelpContentModelPrivate::createContents(const ItemFutureProvider &futureProvider)
{
    const bool wasRunning = bool(watcher);
    watcher.reset(new QFutureWatcher<std::shared_ptr<QHelpContentItem>>);
    QObject::connect(watcher.get(), &QFutureWatcherBase::finished, q, [this] {
        if (!watcher->isCanceled()) {
            const std::shared_ptr<QHelpContentItem> result = watcher->result();
            if (result && result.get()) {
                q->beginResetModel();
                rootItem = result;
                q->endResetModel();
            }
        }
        watcher.release()->deleteLater();
        emit q->contentsCreated();
    });
    watcher->setFuture(futureProvider());

    if (wasRunning)
        return;

    if (rootItem) {
        q->beginResetModel();
        rootItem.reset();
        q->endResetModel();
    }
    emit q->contentsCreationStarted();
}
#endif

/*!
    \class QHelpContentModel
    \inmodule QtHelp
    \brief The QHelpContentModel class provides a model that supplies content to views.
    \since 4.4
*/

/*!
    \fn void QHelpContentModel::contentsCreationStarted()

    This signal is emitted when the creation of the contents has
    started. The current contents are invalid from this point on
    until the signal contentsCreated() is emitted.

    \sa isCreatingContents()
*/

/*!
    \fn void QHelpContentModel::contentsCreated()

    This signal is emitted when the contents have been created.
*/

QHelpContentModel::QHelpContentModel(QHelpEngineCore *helpEngine)
    : QAbstractItemModel(helpEngine)
    , d(new QHelpContentModelPrivate{this, helpEngine})
{}

/*!
    Destroys the help content model.
*/
QHelpContentModel::~QHelpContentModel()
{
    delete d;
}

/*!
    \since 6.8

    Creates new contents by querying the help system for contents specified for the current filter.
*/
void QHelpContentModel::createContentsForCurrentFilter()
{
#if QT_CONFIG(future)
    d->createContents([this] { return d->helpEngine->requestContentForCurrentFilter(); });
#endif
}

/*!
    Creates new contents by querying the help system
    for contents specified for the custom \a filter name.
*/
void QHelpContentModel::createContents(const QString &filter)
{
#if QT_CONFIG(future)
    d->createContents([this, filter] { return d->helpEngine->requestContent(filter); });
#endif
}

// TODO: Remove me
void QHelpContentModel::insertContents()
{}

/*!
    Returns true if the contents are currently rebuilt, otherwise
    false.
*/
bool QHelpContentModel::isCreatingContents() const
{
#if QT_CONFIG(future)
    return bool(d->watcher);
#else
    return false;
#endif
}

/*!
    Returns the help content item at the model index position
    \a index.
*/
QHelpContentItem *QHelpContentModel::contentItemAt(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<QHelpContentItem *>(index.internalPointer())
                           : d->rootItem.get();
}

/*!
    Returns the index of the item in the model specified by
    the given \a row, \a column and \a parent index.
*/
QModelIndex QHelpContentModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!d->rootItem)
        return {};

    QHelpContentItem *parentItem = contentItemAt(parent);
    QHelpContentItem *item = parentItem->child(row);
    if (!item)
        return {};
    return createIndex(row, column, item);
}

/*!
    Returns the parent of the model item with the given
    \a index, or QModelIndex() if it has no parent.
*/
QModelIndex QHelpContentModel::parent(const QModelIndex &index) const
{
    QHelpContentItem *item = contentItemAt(index);
    if (!item)
        return {};

    QHelpContentItem *parentItem = static_cast<QHelpContentItem*>(item->parent());
    if (!parentItem)
        return {};

    QHelpContentItem *grandparentItem = static_cast<QHelpContentItem*>(parentItem->parent());
    if (!grandparentItem)
        return {};

    const int row = grandparentItem->childPosition(parentItem);
    return createIndex(row, index.column(), parentItem);
}

/*!
    Returns the number of rows under the given \a parent.
*/
int QHelpContentModel::rowCount(const QModelIndex &parent) const
{
    QHelpContentItem *parentItem = contentItemAt(parent);
    if (parentItem)
        return parentItem->childCount();
    return 0;
}

/*!
    Returns the number of columns under the given \a parent. Currently returns always 1.
*/
int QHelpContentModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

/*!
    Returns the data stored under the given \a role for
    the item referred to by the \a index.
*/
QVariant QHelpContentModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        QHelpContentItem *item = contentItemAt(index);
        if (item)
            return item->title();
    }
    return {};
}

/*!
    \class QHelpContentWidget
    \inmodule QtHelp
    \brief The QHelpContentWidget class provides a tree view for displaying help content model items.
    \since 4.4
*/

/*!
    \fn void QHelpContentWidget::linkActivated(const QUrl &link)

    This signal is emitted when a content item is activated and
    its associated \a link should be shown.
*/

QHelpContentWidget::QHelpContentWidget()
{
    header()->hide();
    setUniformRowHeights(true);
    connect(this, &QAbstractItemView::activated, this, &QHelpContentWidget::showLink);
}

/*!
    Returns the index of the content item with the \a link.
    An invalid index is returned if no such an item exists.
*/
QModelIndex QHelpContentWidget::indexOf(const QUrl &link)
{
    QHelpContentModel *contentModel = qobject_cast<QHelpContentModel*>(model());
    if (!contentModel || link.scheme() != "qthelp"_L1)
        return {};

    m_syncIndex = {};
    for (int i = 0; i < contentModel->rowCount(); ++i) {
        QHelpContentItem *itm = contentModel->contentItemAt(contentModel->index(i, 0));
        if (itm && itm->url().host() == link.host()) {
            if (searchContentItem(contentModel, contentModel->index(i, 0), QDir::cleanPath(link.path())))
                return m_syncIndex;
        }
    }
    return {};
}

bool QHelpContentWidget::searchContentItem(QHelpContentModel *model, const QModelIndex &parent,
                                           const QString &cleanPath)
{
    QHelpContentItem *parentItem = model->contentItemAt(parent);
    if (!parentItem)
        return false;

    if (QDir::cleanPath(parentItem->url().path()) == cleanPath) {
        m_syncIndex = parent;
        return true;
    }

    for (int i = 0; i < parentItem->childCount(); ++i) {
        if (searchContentItem(model, model->index(i, 0, parent), cleanPath))
            return true;
    }
    return false;
}

void QHelpContentWidget::showLink(const QModelIndex &index)
{
    QHelpContentModel *contentModel = qobject_cast<QHelpContentModel*>(model());
    if (!contentModel)
        return;

    QHelpContentItem *item = contentModel->contentItemAt(index);
    if (!item)
        return;
    QUrl url = item->url();
    if (url.isValid())
        emit linkActivated(url);
}

QT_END_NAMESPACE
