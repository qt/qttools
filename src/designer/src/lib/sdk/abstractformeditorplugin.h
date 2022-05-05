/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef ABSTRACTFORMEDITORPLUGIN_H
#define ABSTRACTFORMEDITORPLUGIN_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QAction;

class QDESIGNER_SDK_EXPORT QDesignerFormEditorPluginInterface
{
public:
    Q_DISABLE_COPY_MOVE(QDesignerFormEditorPluginInterface)

    QDesignerFormEditorPluginInterface() = default;
    virtual ~QDesignerFormEditorPluginInterface() = default;

    virtual bool isInitialized() const = 0;
    virtual void initialize(QDesignerFormEditorInterface *core) = 0;
    virtual QAction *action() const = 0;

    virtual QDesignerFormEditorInterface *core() const = 0;
};
Q_DECLARE_INTERFACE(QDesignerFormEditorPluginInterface, "org.qt-project.Qt.Designer.QDesignerFormEditorPluginInterface")

QT_END_NAMESPACE

#endif // ABSTRACTFORMEDITORPLUGIN_H
