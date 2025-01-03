// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SIGNALSLOTEDITOR_TOOL_H
#define SIGNALSLOTEDITOR_TOOL_H

#include "signalsloteditor_global.h"
#include "signalsloteditor.h"

#include <QtCore/qpointer.h>
#include <QtDesigner/abstractformwindowtool.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class SignalSlotEditor;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditorTool: public QDesignerFormWindowToolInterface
{
    Q_OBJECT
public:
    explicit SignalSlotEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent = nullptr);
    ~SignalSlotEditorTool() override;

    QDesignerFormEditorInterface *core() const override;
    QDesignerFormWindowInterface *formWindow() const override;

    QWidget *editor() const override;

    QAction *action() const override;

    void activated() override;
    void deactivated() override;

    bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) override;

    void saveToDom(DomUI *ui, QWidget *mainContainer) override;
    void loadFromDom(DomUI *ui, QWidget *mainContainer) override;

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<qdesigner_internal::SignalSlotEditor> m_editor;
    QAction *m_action;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTEDITOR_TOOL_H
