// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SIGNALSLOTEDITOR_PLUGIN_H
#define SIGNALSLOTEDITOR_PLUGIN_H

#include "signalsloteditor_global.h"

#include <QtDesigner/abstractformeditorplugin.h>

#include <QtCore/qpointer.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class SignalSlotEditorTool;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditorPlugin: public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerFormEditorPluginInterface" FILE "signalsloteditor.json")
    Q_INTERFACES(QDesignerFormEditorPluginInterface)
public:
    SignalSlotEditorPlugin();
    ~SignalSlotEditorPlugin() override;

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
    QHash<QDesignerFormWindowInterface*, SignalSlotEditorTool*> m_tools;
    bool m_initialized = false;
    QAction *m_action = nullptr;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTEDITOR_PLUGIN_H
