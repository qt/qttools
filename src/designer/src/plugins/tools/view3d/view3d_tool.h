// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef VIEW3D_TOOL_H
#define VIEW3D_TOOL_H

#include "view3d_global.h"
#include "view3d.h"

#include <QtDesigner/qdesignerformwindowtoolinterface.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class VIEW3D_EXPORT QView3DTool : public QDesignerFormWindowToolInterface
{
    Q_OBJECT

public:
    explicit QView3DTool(QDesignerFormWindowInterface *formWindow, QObject *parent = 0);
    QDesignerFormEditorInterface *core() const override;
    QDesignerFormWindowInterface *formWindow() const override;
    QWidget *editor() const override;

    QAction *action() const override;

    virtual void activated();
    virtual void deactivated();

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) override;

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<QView3D> m_editor;
    QAction *m_action;
};

QT_END_NAMESPACE

#endif // VIEW3D_TOOL_H
