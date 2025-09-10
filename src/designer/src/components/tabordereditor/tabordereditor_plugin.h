// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TABORDEREDITOR_PLUGIN_H
#define TABORDEREDITOR_PLUGIN_H

#include "tabordereditor_global.h"

#include <QtDesigner/abstractformeditorplugin.h>

#include <QtCore/qpointer.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class TabOrderEditorTool;

class QT_TABORDEREDITOR_EXPORT TabOrderEditorPlugin: public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerFormEditorPluginInterface" FILE "tabordereditor.json")
    Q_INTERFACES(QDesignerFormEditorPluginInterface)
public:
    TabOrderEditorPlugin();
    ~TabOrderEditorPlugin() override;

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
    QHash<QDesignerFormWindowInterface*, TabOrderEditorTool*> m_tools;
    bool m_initialized = false;
    QAction *m_action = nullptr;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TABORDEREDITOR_PLUGIN_H
