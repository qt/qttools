// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpcontentitem.h"

#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QHelpContentItemPrivate
{
public:
    QString title;
    QUrl link;
    QHelpContentItem *parent;
    QList<QHelpContentItem *> childItems = {};
};

/*!
    \class QHelpContentItem
    \inmodule QtHelp
    \brief The QHelpContentItem class provides an item for use with QHelpContentModel.
    \since 4.4
*/

QHelpContentItem::QHelpContentItem(const QString &name, const QUrl &link, QHelpContentItem *parent)
    : d(new QHelpContentItemPrivate{name, link, parent})
{
    if (parent)
        parent->d->childItems.append(this);
}

/*!
    Destroys the help content item.
*/
QHelpContentItem::~QHelpContentItem()
{
    qDeleteAll(d->childItems);
    delete d;
}

/*!
    Returns the child of the content item in the give \a row.

    \sa parent()
*/
QHelpContentItem *QHelpContentItem::child(int row) const
{
    return d->childItems.value(row);
}

/*!
    Returns the number of child items.
*/
int QHelpContentItem::childCount() const
{
    return d->childItems.size();
}

/*!
    Returns the row of this item from its parents view.
*/
int QHelpContentItem::row() const
{
    // TODO: Optimize by keeping the index internally.
    return d->parent ? d->parent->d->childItems.indexOf(const_cast<QHelpContentItem*>(this)) : 0;
}

/*!
    Returns the title of the content item.
*/
QString QHelpContentItem::title() const
{
    return d->title;
}

/*!
    Returns the URL of this content item.
*/
QUrl QHelpContentItem::url() const
{
    return d->link;
}

/*!
    Returns the parent content item.
*/
QHelpContentItem *QHelpContentItem::parent() const
{
    return d->parent;
}

/*!
    Returns the position of a given \a child.
*/
int QHelpContentItem::childPosition(QHelpContentItem *child) const
{
    return d->childItems.indexOf(child);
}

QT_END_NAMESPACE
