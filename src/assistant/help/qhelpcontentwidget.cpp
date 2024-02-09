// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpcontentwidget.h"
#include "qhelpcollectionhandler_p.h"
#include "qhelpenginecore.h"
#include "qhelpfilterengine.h"

#include <QtCore/qdir.h>
#include <QtCore/qmutex.h>
#include <QtCore/qthread.h>
#include <QtWidgets/qheaderview.h>

QT_BEGIN_NAMESPACE

class QHelpContentProvider : public QThread
{
    Q_OBJECT
public:
    QHelpContentProvider(QHelpEngineCore *helpEngine);
    ~QHelpContentProvider() override;
    void collectContentsForCurrentFilter();
    void collectContents(const QString &customFilterName);
    void stopCollecting();
    QHelpContentItem *takeContentItem();

private:
    void run() override;

    QHelpEngineCore *m_helpEngine;
    QString m_currentFilter;
    QStringList m_filterAttributes;
    QString m_collectionFile;
    QHelpContentItem *m_rootItem = nullptr;
    QMutex m_mutex;
    bool m_usesFilterEngine = false;
    bool m_abort = false;
};

class QHelpContentModelPrivate
{
public:
    QHelpContentItem *rootItem = nullptr;
    QHelpContentProvider *qhelpContentProvider;
};

QHelpContentProvider::QHelpContentProvider(QHelpEngineCore *helpEngine)
    : QThread(helpEngine)
    , m_helpEngine(helpEngine)
{}

QHelpContentProvider::~QHelpContentProvider()
{
    stopCollecting();
}

void QHelpContentProvider::collectContentsForCurrentFilter()
{
    m_mutex.lock();
    m_currentFilter = m_helpEngine->filterEngine()->activeFilter();
    m_filterAttributes = m_helpEngine->filterAttributes(m_helpEngine->legacyCurrentFilterName());
    m_collectionFile = m_helpEngine->collectionFile();
    m_usesFilterEngine = m_helpEngine->usesFilterEngine();
    m_mutex.unlock();

    if (isRunning())
        stopCollecting();
    start(LowPriority);
}

void QHelpContentProvider::collectContents(const QString &customFilterName)
{
    m_mutex.lock();
    m_currentFilter = customFilterName;
    m_filterAttributes = m_helpEngine->filterAttributes(customFilterName);
    m_collectionFile = m_helpEngine->collectionFile();
    m_usesFilterEngine = m_helpEngine->usesFilterEngine();
    m_mutex.unlock();

    if (isRunning())
        stopCollecting();
    start(LowPriority);
}

void QHelpContentProvider::stopCollecting()
{
    if (isRunning()) {
        m_mutex.lock();
        m_abort = true;
        m_mutex.unlock();
        wait();
        // we need to force-set m_abort to false, because the thread might either have
        // finished between the isRunning() check and the "m_abort = true" above, or the
        // isRunning() check might already happen after the "m_abort = false" in the run() method,
        // either way never resetting m_abort to false from within the run() method
        m_abort = false;
    }
    delete m_rootItem;
    m_rootItem = nullptr;
}

QHelpContentItem *QHelpContentProvider::takeContentItem()
{
    QMutexLocker locker(&m_mutex);
    QHelpContentItem *content = m_rootItem;
    m_rootItem = nullptr;
    return content;
}


static QUrl constructUrl(const QString &namespaceName,
                         const QString &folderName,
                         const QString &relativePath)
{
    const int idx = relativePath.indexOf(QLatin1Char('#'));
    const QString &rp = idx < 0 ? relativePath : relativePath.left(idx);
    const QString anchor = idx < 0 ? QString() : relativePath.mid(idx + 1);
    return QHelpCollectionHandler::buildQUrl(namespaceName, folderName, rp, anchor);
}

void QHelpContentProvider::run()
{
    m_mutex.lock();
    const QString currentFilter = m_currentFilter;
    const QStringList attributes = m_filterAttributes;
    const QString collectionFile = m_collectionFile;
    const bool usesFilterEngine = m_usesFilterEngine;
    delete m_rootItem;
    m_rootItem = nullptr;
    m_mutex.unlock();

    if (collectionFile.isEmpty())
        return;

    QHelpCollectionHandler collectionHandler(collectionFile);
    if (!collectionHandler.openCollectionFile())
        return;

    QHelpContentItem * const rootItem = new QHelpContentItem(QString(), QString(), nullptr);

    const QList<QHelpCollectionHandler::ContentsData> result = usesFilterEngine
            ? collectionHandler.contentsForFilter(currentFilter)
            : collectionHandler.contentsForFilter(attributes);

    for (const auto &contentsData : result) {
        m_mutex.lock();
        if (m_abort) {
            delete rootItem;
            m_abort = false;
            m_mutex.unlock();
            return;
        }
        m_mutex.unlock();

        const QString namespaceName = contentsData.namespaceName;
        const QString folderName = contentsData.folderName;
        for (const QByteArray &contents : contentsData.contentsList)  {
            if (contents.isEmpty())
                continue;

            QList<QHelpContentItem *> stack;
            QDataStream s(contents);
            while (true) {
                int depth = 0;
                QString link, title;
                s >> depth;
                s >> link;
                s >> title;
                if (title.isEmpty())
                    break;

// The example input (depth, link, title):
//
// 0 "graphicaleffects5.html" "Qt 5 Compatibility APIs: Qt Graphical Effects"
// 1 "qtgraphicaleffects5-index.html" "QML Types"
// 2 "qml-qt5compat-graphicaleffects-blend.html" "Blend Type Reference"
// 3 "qml-qt5compat-graphicaleffects-blend-members.html" "List of all members"
// 2 "qml-qt5compat-graphicaleffects-brightnesscontrast.html" "BrightnessContrast Type Reference"
//
// Thus, the valid order of depths is:
// 1. Whenever the item's depth is < 0, we insert the item as its depth is 0.
// 2. The first item's depth must be 0, otherwise we insert the item as its depth is 0.
// 3. When the previous depth was N, the next depth must be in range [0, N+1] inclusively.
//    If next item's depth is M > N+1, we insert the item as its depth is N+1.

                if (depth <= 0) {
                    stack.clear();
                } else if (depth < stack.size()) {
                    stack = stack.sliced(0, depth);
                } else if (depth > stack.size()) {
                    // Fill the gaps with the last item from the stack (or with the root).
                    // This branch handles the case when depths are broken, e.g. 0, 2, 2, 1.
                    // In this case, the 1st item is a root, and 2nd - 4th are all direct
                    // children of the 1st.
                    QHelpContentItem *substituteItem = stack.isEmpty() ? rootItem : stack.constLast();
                    while (depth > stack.size())
                        stack.append(substituteItem);
                }

                const QUrl url = constructUrl(namespaceName, folderName, link);
                QHelpContentItem *parent = stack.isEmpty() ? rootItem : stack.constLast();
                stack.push_back(new QHelpContentItem(title, url, parent));
            }
        }
    }

    m_mutex.lock();
    m_rootItem = rootItem;
    m_abort = false;
    m_mutex.unlock();
}

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
    , d(new QHelpContentModelPrivate)
{
    d->qhelpContentProvider = new QHelpContentProvider(helpEngine);
    connect(d->qhelpContentProvider, &QThread::finished, this, &QHelpContentModel::insertContents);
}

/*!
    Destroys the help content model.
*/
QHelpContentModel::~QHelpContentModel()
{
    delete d->rootItem;
    delete d;
}

/*!
    \since 6.8

    Creates new contents by querying the help system for contents specified for the current filter.
*/
void QHelpContentModel::createContentsForCurrentFilter()
{
    const bool running = d->qhelpContentProvider->isRunning();
    d->qhelpContentProvider->collectContentsForCurrentFilter();
    if (running)
        return;

    if (d->rootItem) {
        beginResetModel();
        delete d->rootItem;
        d->rootItem = nullptr;
        endResetModel();
    }
    emit contentsCreationStarted();
}

/*!
    Creates new contents by querying the help system
    for contents specified for the \a customFilterName.
*/
void QHelpContentModel::createContents(const QString &customFilterName)
{
    const bool running = d->qhelpContentProvider->isRunning();
    d->qhelpContentProvider->collectContents(customFilterName);
    if (running)
        return;

    if (d->rootItem) {
        beginResetModel();
        delete d->rootItem;
        d->rootItem = nullptr;
        endResetModel();
    }
    emit contentsCreationStarted();
}

void QHelpContentModel::insertContents()
{
    if (d->qhelpContentProvider->isRunning())
        return;

    QHelpContentItem * const newRootItem = d->qhelpContentProvider->takeContentItem();
    if (!newRootItem)
        return;
    beginResetModel();
    delete d->rootItem;
    d->rootItem = newRootItem;
    endResetModel();
    emit contentsCreated();
}

/*!
    Returns true if the contents are currently rebuilt, otherwise
    false.
*/
bool QHelpContentModel::isCreatingContents() const
{
    return d->qhelpContentProvider->isRunning();
}

/*!
    Returns the help content item at the model index position
    \a index.
*/
QHelpContentItem *QHelpContentModel::contentItemAt(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<QHelpContentItem*>(index.internalPointer()) : d->rootItem;
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

    int row = grandparentItem->childPosition(parentItem);
    return createIndex(row, index.column(), parentItem);
}

/*!
    Returns the number of rows under the given \a parent.
*/
int QHelpContentModel::rowCount(const QModelIndex &parent) const
{
    QHelpContentItem *parentItem = contentItemAt(parent);
    if (!parentItem)
        return 0;
    return parentItem->childCount();
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
    if (!contentModel || link.scheme() != QLatin1String("qthelp"))
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

#include "qhelpcontentwidget.moc"
