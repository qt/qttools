// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SERVICESFILTERPROXYMODEL_H
#define SERVICESFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class ServicesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ServicesProxyModel(QObject *parent = nullptr);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // SERVICESFILTERPROXYMODEL_H
