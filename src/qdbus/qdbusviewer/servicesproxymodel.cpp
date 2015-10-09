/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
            int number1 = s1.midRef(3).toInt();
            int number2 = s2.midRef(3).toInt();
            return number1 < number2;
        } else {
            return s1.compare(s2, Qt::CaseInsensitive) < 0;
        }
    } else {
        return isNumber2;
    }
}
