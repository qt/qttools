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

#endif // VIEW3D_TOOL_H

QT_END_NAMESPACE
