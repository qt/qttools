// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpindexwidget.h"
#include "qhelpenginecore.h"
#include "qhelplink.h"

#if QT_CONFIG(future)
#include <QtCore/qfuturewatcher.h>
#endif

QT_BEGIN_NAMESPACE

class QHelpIndexModelPrivate
{
#if QT_CONFIG(future)
    using FutureProvider = std::function<QFuture<QStringList>()>;

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
    void createIndex(const FutureProvider &futureProvider);
#endif

    QHelpIndexModel *q = nullptr;
    QHelpEngineCore *helpEngine = nullptr;
    QStringList indices = {};
#if QT_CONFIG(future)
    std::unique_ptr<QFutureWatcher<QStringList>, WatcherDeleter> watcher = {};
#endif
};

#if QT_CONFIG(future)
void QHelpIndexModelPrivate::createIndex(const FutureProvider &futureProvider)
{
    const bool wasRunning = bool(watcher);
    watcher.reset(new QFutureWatcher<QStringList>);
    QObject::connect(watcher.get(), &QFutureWatcherBase::finished, q, [this] {
        if (!watcher->isCanceled()) {
            indices = watcher->result();
            q->filter({});
        }
        watcher.release()->deleteLater();
        emit q->indexCreated();
    });
    watcher->setFuture(futureProvider());

    if (wasRunning)
        return;

    indices.clear();
    q->filter({});
    emit q->indexCreationStarted();
}
#endif

/*!
    \class QHelpIndexModel
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpIndexModel class provides a model that
    supplies index keywords to views.
*/

/*!
    \fn void QHelpIndexModel::indexCreationStarted()

    This signal is emitted when the creation of a new index
    has started. The current index is invalid from this
    point on until the signal indexCreated() is emitted.

    \sa isCreatingIndex()
*/

/*!
    \fn void QHelpIndexModel::indexCreated()

    This signal is emitted when the index has been created.
*/

QHelpIndexModel::QHelpIndexModel(QHelpEngineCore *helpEngine)
    : QStringListModel(helpEngine)
    , d(new QHelpIndexModelPrivate{this, helpEngine})
{}

QHelpIndexModel::~QHelpIndexModel()
{
    delete d;
}

/*!
    \since 6.8

    Creates a new index by querying the help system for keywords for the current filter.
*/
void QHelpIndexModel::createIndexForCurrentFilter()
{
#if QT_CONFIG(future)
    d->createIndex([this] { return d->helpEngine->requestIndexForCurrentFilter(); });
#endif
}

/*!
    Creates a new index by querying the help system for
    keywords for the specified custom \a filter name.
*/
void QHelpIndexModel::createIndex(const QString &filter)
{
#if QT_CONFIG(future)
    d->createIndex([this, filter] { return d->helpEngine->requestIndex(filter); });
#endif
}

// TODO: Remove me
void QHelpIndexModel::insertIndices()
{}

/*!
    Returns true if the index is currently built up, otherwise
    false.
*/
bool QHelpIndexModel::isCreatingIndex() const
{
#if QT_CONFIG(future)
    return bool(d->watcher);
#else
    return false;
#endif
}

/*!
    \since 5.15

    Returns the associated help engine that manages this model.
*/
QHelpEngineCore *QHelpIndexModel::helpEngine() const
{
    return d->helpEngine;
}

/*!
    Filters the indices and returns the model index of the best
    matching keyword. In a first step, only the keywords containing
    \a filter are kept in the model's index list. Analogously, if
    \a wildcard is not empty, only the keywords matched are left
    in the index list. In a second step, the best match is
    determined and its index model returned. When specifying a
    wildcard expression, the \a filter string is used to
    search for the best match.
*/
QModelIndex QHelpIndexModel::filter(const QString &filter, const QString &wildcard)
{
    if (filter.isEmpty()) {
        setStringList(d->indices);
        return index(-1, 0, {});
    }

    using Checker = std::function<bool(const QString &)>;
    const auto checkIndices = [this, filter](const Checker &checker) {
        QStringList filteredList;
        int goodMatch = -1;
        int perfectMatch = -1;
        for (const QString &index : std::as_const(d->indices)) {
            if (checker(index)) {
                filteredList.append(index);
                if (perfectMatch == -1 && index.startsWith(filter, Qt::CaseInsensitive)) {
                    if (goodMatch == -1)
                        goodMatch = filteredList.size() - 1;
                    if (filter.size() == index.size())
                        perfectMatch = filteredList.size() - 1;
                } else if (perfectMatch > -1 && index == filter) {
                    perfectMatch = filteredList.size() - 1;
                }
            }
        }
        setStringList(filteredList);
        return perfectMatch >= 0 ? perfectMatch : qMax(0, goodMatch);
    };

    int perfectMatch = -1;
    if (!wildcard.isEmpty()) {
        const auto re = QRegularExpression::wildcardToRegularExpression(wildcard,
                        QRegularExpression::UnanchoredWildcardConversion);
        const QRegularExpression regExp(re, QRegularExpression::CaseInsensitiveOption);
        perfectMatch = checkIndices([regExp](const QString &index) {
            return index.contains(regExp);
        });
    } else {
        perfectMatch = checkIndices([filter](const QString &index) {
            return index.contains(filter, Qt::CaseInsensitive);
        });
    }
    return index(perfectMatch, 0, {});
}

/*!
    \class QHelpIndexWidget
    \inmodule QtHelp
    \since 4.4
    \brief The QHelpIndexWidget class provides a list view
    displaying the QHelpIndexModel.
*/

/*!
    \fn void QHelpIndexWidget::linkActivated(const QUrl &link,
        const QString &keyword)

    \deprecated

    Use documentActivated() instead.

    This signal is emitted when an item is activated and its
    associated \a link should be shown. To know where the link
    belongs to, the \a keyword is given as a second parameter.
*/

/*!
    \fn void QHelpIndexWidget::documentActivated(const QHelpLink &document,
        const QString &keyword)

    \since 5.15

    This signal is emitted when an item is activated and its
    associated \a document should be shown. To know where the link
    belongs to, the \a keyword is given as a second parameter.
*/

/*!
    \fn void QHelpIndexWidget::documentsActivated(const QList<QHelpLink> &documents,
        const QString &keyword)

    \since 5.15

    This signal is emitted when the item representing the \a keyword
    is activated and the item has more than one document associated.
    The \a documents consist of the document titles and their URLs.
*/

QHelpIndexWidget::QHelpIndexWidget()
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setUniformItemSizes(true);
    connect(this, &QAbstractItemView::activated, this, &QHelpIndexWidget::showLink);
}

void QHelpIndexWidget::showLink(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QHelpIndexModel *indexModel = qobject_cast<QHelpIndexModel*>(model());
    if (!indexModel)
        return;

    const QVariant &v = indexModel->data(index, Qt::DisplayRole);
    const QString name = v.isValid() ? v.toString() : QString();

    const QList<QHelpLink> &docs = indexModel->helpEngine()->documentsForKeyword(name);
    if (docs.size() > 1) {
        emit documentsActivated(docs, name);
#if QT_DEPRECATED_SINCE(5, 15)
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
        QMultiMap<QString, QUrl> links;
        for (const auto &doc : docs)
            links.insert(doc.title, doc.url);
        emit linksActivated(links, name);
        QT_WARNING_POP
#endif
    } else if (!docs.isEmpty()) {
        emit documentActivated(docs.first(), name);
#if QT_DEPRECATED_SINCE(5, 15)
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_DEPRECATED
        emit linkActivated(docs.first().url, name);
        QT_WARNING_POP
#endif
    }
}

/*!
    Activates the current item which will result eventually in
    the emitting of a linkActivated() or linksActivated()
    signal.
*/
void QHelpIndexWidget::activateCurrentItem()
{
    showLink(currentIndex());
}

/*!
    Filters the indices according to \a filter or \a wildcard.
    The item with the best match is set as current item.

    \sa QHelpIndexModel::filter()
*/
void QHelpIndexWidget::filterIndices(const QString &filter, const QString &wildcard)
{
    QHelpIndexModel *indexModel = qobject_cast<QHelpIndexModel *>(model());
    if (!indexModel)
        return;
    const QModelIndex &idx = indexModel->filter(filter, wildcard);
    if (idx.isValid())
        setCurrentIndex(idx);
}

QT_END_NAMESPACE
