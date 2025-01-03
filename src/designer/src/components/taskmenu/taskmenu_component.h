// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TASKMENU_COMPONENT_H
#define TASKMENU_COMPONENT_H

#include "taskmenu_global.h"
#include <QtDesigner/taskmenu.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class QT_TASKMENU_EXPORT TaskMenuComponent: public QObject
{
    Q_OBJECT
public:
    explicit TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent = nullptr);
    ~TaskMenuComponent() override;

    QDesignerFormEditorInterface *core() const;

private:
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TASKMENU_COMPONENT_H
