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

#ifndef QMLUTILS_H
#define QMLUTILS_H

#include "utils.h"

#include <QStringList>

QT_BEGIN_NAMESPACE

QString findQmlDirectory(Platform platform, const QString &startDirectoryName);

struct QmlImportScanResult {
    struct Module {
        QString installPath(const QString &root) const;

        QString name;
        QString className;
        QString sourcePath;
        QString relativePath;
    };

    void append(const QmlImportScanResult &other);

    bool ok = false;
    QList<Module> modules;
    QStringList plugins;
};

bool operator==(const QmlImportScanResult::Module &m1, const QmlImportScanResult::Module &m2);

QmlImportScanResult runQmlImportScanner(const QString &directory, const QStringList &qmlImportPaths,
                                        bool usesWidgets, int platform, DebugMatchMode debugMatchMode,
                                        QString *errorMessage);

QT_END_NAMESPACE

#endif // QMLUTILS_H
