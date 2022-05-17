// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MULTIPAGEWIDGETEXTENSIONFACTORY_H
#define MULTIPAGEWIDGETEXTENSIONFACTORY_H

#include <QtDesigner/QExtensionFactory>

QT_BEGIN_NAMESPACE
class QExtensionManager;
QT_END_NAMESPACE

//! [0]
class MultiPageWidgetExtensionFactory: public QExtensionFactory
{
    Q_OBJECT

public:
    explicit MultiPageWidgetExtensionFactory(QExtensionManager *parent = nullptr);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const override;
};
//! [0]

#endif
