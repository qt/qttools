// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMEDITOR_H
#define FORMEDITOR_H

#include "formeditor_global.h"

#include <QtDesigner/abstractformeditor.h>

QT_BEGIN_NAMESPACE

class QObject;

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormEditor: public QDesignerFormEditorInterface
{
    Q_OBJECT
public:
    FormEditor(QObject *parent = nullptr);
    FormEditor(const QStringList &pluginPaths,
               QObject *parent = nullptr);
    ~FormEditor() override;
public slots:
    void slotQrcFileChangedExternally(const QString &path);
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMEDITOR_H
