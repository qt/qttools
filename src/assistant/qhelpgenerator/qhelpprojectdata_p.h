/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
