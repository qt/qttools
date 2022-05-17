// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ITEMVIEWFINDWIDGET_H
#define ITEMVIEWFINDWIDGET_H

#include "abstractfindwidget.h"

#include <QModelIndex>

QT_BEGIN_NAMESPACE

class QAbstractItemView;

class ItemViewFindWidget : public AbstractFindWidget
{
    Q_OBJECT

public:
    explicit ItemViewFindWidget(FindFlags flags = FindFlags(), QWidget *parent = 0);

    QAbstractItemView *itemView() const
    { return m_itemView; }

    void setItemView(QAbstractItemView *itemView);

protected:
    void deactivate() override;
    void find(const QString &textToFind, bool skipCurrent,
              bool backward, bool *found, bool *wrapped) override;

private:
    QModelIndex findHelper(const QString &textToFind, bool skipCurrent, bool backward,
        QModelIndex parent, int row, int column);

    QAbstractItemView *m_itemView;
};

QT_END_NAMESPACE

#endif  // ITEMVIEWFINDWIDGET_H
