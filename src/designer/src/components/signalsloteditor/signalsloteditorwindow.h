// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SIGNALSLOTEDITORWINDOW_H
#define SIGNALSLOTEDITORWINDOW_H

#include <QtCore/qpointer.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QModelIndex;
class QSortFilterProxyModel;
class QTreeView;
class QToolButton;

namespace qdesigner_internal {

class SignalSlotEditor;
class ConnectionModel;
class Connection;

class SignalSlotEditorWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SignalSlotEditorWindow(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);

public slots:
    void setActiveFormWindow(QDesignerFormWindowInterface *form);

private slots:
    void updateDialogSelection(Connection *con);
    void updateEditorSelection(const QModelIndex &index);

    void objectNameChanged(QDesignerFormWindowInterface *formWindow, QObject *object, const QString &newName, const QString &oldName);

    void addConnection();
    void removeConnection();
    void updateUi();
    void resizeColumns();

private:
    QTreeView *m_view;
    QPointer<SignalSlotEditor> m_editor;
    QToolButton *m_add_button, *m_remove_button;
    QDesignerFormEditorInterface *m_core;
    ConnectionModel *m_model;
    QSortFilterProxyModel *m_proxy_model;
    bool m_handling_selection_change;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTEDITORWINDOW_H
