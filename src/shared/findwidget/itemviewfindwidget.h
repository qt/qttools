/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

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
