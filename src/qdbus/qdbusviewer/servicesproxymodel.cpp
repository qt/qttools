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
******************************************************************************/

#include "servicesproxymodel.h"

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

    const bool isNumber1 = s1.startsWith(QLatin1String(":1."));
    const bool isNumber2 = s2.startsWith(QLatin1String(":1."));
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
