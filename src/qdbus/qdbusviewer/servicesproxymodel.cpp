// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "servicesproxymodel.h"

using namespace Qt::StringLiterals;

ServicesProxyModel::ServicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

QVariant ServicesProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section != 0)
        return QVariant();

    return tr("Services");
}


bool ServicesProxyModel::lessThan(const QModelIndex &left,
                                  const QModelIndex &right) const
{
    QString s1 = sourceModel()->data(left).toString();
    QString s2 = sourceModel()->data(right).toString();

    const bool isNumber1 = s1.startsWith(":1."_L1);
    const bool isNumber2 = s2.startsWith(":1."_L1);
    if (isNumber1 == isNumber2) {
        if (isNumber1) {
            int number1 = QStringView{s1}.mid(3).toInt();
            int number2 = QStringView{s2}.mid(3).toInt();
            return number1 < number2;
        } else {
            return s1.compare(s2, Qt::CaseInsensitive) < 0;
        }
    } else {
        return isNumber2;
    }
}
