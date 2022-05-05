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

#ifndef BUDDYEDITOR_PLUGIN_H
#define BUDDYEDITOR_PLUGIN_H

#include "buddyeditor_global.h"

#include <QtDesigner/abstractformeditorplugin.h>

#include <QtCore/qpointer.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class BuddyEditorTool;

class QT_BUDDYEDITOR_EXPORT BuddyEditorPlugin: public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerFormEditorPluginInterface" FILE "buddyeditor.json")
    Q_INTERFACES(QDesignerFormEditorPluginInterface)
public:
    BuddyEditorPlugin();
    ~BuddyEditorPlugin() override;

    bool isInitialized() const override;
    void initialize(QDesignerFormEditorInterface *core) override;
    QAction *action() const override;

    QDesignerFormEditorInterface *core() const override;

public slots:
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);

private slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow);
    void removeFormWindow(QDesignerFormWindowInterface *formWindow);

private:
    QPointer<QDesignerFormEditorInterface> m_core;
    QHash<QDesignerFormWindowInterface*, BuddyEditorTool*> m_tools;
    bool m_initialized = false;
    QAction *m_action = nullptr;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // BUDDYEDITOR_PLUGIN_H
