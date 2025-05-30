// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELPPROJECTDATA_H
#define QHELPPROJECTDATA_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include "qhelpdatainterface_p.h"

QT_BEGIN_NAMESPACE

class QHelpProjectDataPrivate;

class QHelpProjectData
{
public:
    QHelpProjectData();
    ~QHelpProjectData();

    bool readData(const QString &fileName);
    QString errorMessage() const;

    QString namespaceName() const;
    QString virtualFolder() const;
    QList<QHelpDataCustomFilter> customFilters() const;
    QList<QHelpDataFilterSection> filterSections() const;
    QMap<QString, QVariant> metaData() const;
    QString rootPath() const;

private:
    QHelpProjectDataPrivate *d;
};

QT_END_NAMESPACE

#endif
