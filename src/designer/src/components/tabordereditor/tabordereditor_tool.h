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

#ifndef TABORDEREDITOR_TOOL_H
#define TABORDEREDITOR_TOOL_H

#include "tabordereditor_global.h"

#include <QtCore/qpointer.h>

#include <QtDesigner/abstractformwindowtool.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class TabOrderEditor;

class QT_TABORDEREDITOR_EXPORT TabOrderEditorTool: public QDesignerFormWindowToolInterface
{
    Q_OBJECT
public:
    explicit TabOrderEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent = nullptr);
    ~TabOrderEditorTool() override;

    QDesignerFormEditorInterface *core() const override;
    QDesignerFormWindowInterface *formWindow() const override;

    QWidget *editor() const override;
    QAction *action() const override;

    void activated() override;
    void deactivated() override;

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) override;

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<TabOrderEditor> m_editor;
    QAction *m_action;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TABORDEREDITOR_TOOL_H
