// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BUDDYEDITOR_TOOL_H
#define BUDDYEDITOR_TOOL_H

#include "buddyeditor_global.h"

#include <QtCore/qpointer.h>

#include <QtDesigner/abstractformwindowtool.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class BuddyEditor;

class QT_BUDDYEDITOR_EXPORT BuddyEditorTool: public QDesignerFormWindowToolInterface
{
    Q_OBJECT
public:
    explicit BuddyEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent = nullptr);
    ~BuddyEditorTool() override;

    QDesignerFormEditorInterface *core() const override;
    QDesignerFormWindowInterface *formWindow() const override;

    QWidget *editor() const override;
    QAction *action() const override;

    void activated() override;
    void deactivated() override;

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) override;

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<BuddyEditor> m_editor;
    QAction *m_action;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // BUDDYEDITOR_TOOL_H
