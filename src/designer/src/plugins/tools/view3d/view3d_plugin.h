// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef Q3VIEW3D_PLUGIN_H
#define Q3VIEW3D_PLUGIN_H

#include "view3d_global.h"

#include <QtCore/qlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qpointer.h>
#include <QtDesigner/qdesignerformeditorplugininterface.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QView3DTool;
class QAction;

class VIEW3D_EXPORT QView3DPlugin : public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerFormEditorPluginInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Designer.QDesignerFormEditorPluginInterface")

public:
    QView3DPlugin();
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
    QHash<QDesignerFormWindowInterface*, QView3DTool*> m_tool_list;
    QAction *m_action;
};

QT_END_NAMESPACE

#endif // QVIEW3D_PLUGIN_H
