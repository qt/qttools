// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef VERSIONDIALOG_H
#define VERSIONDIALOG_H

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

class VersionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit VersionDialog(QWidget *parent);
};

QT_END_NAMESPACE

#endif
