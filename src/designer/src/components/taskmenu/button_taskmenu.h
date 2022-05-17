// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BUTTON_TASKMENU_H
#define BUTTON_TASKMENU_H

#include <QtWidgets/qabstractbutton.h>
#include <QtWidgets/qcommandlinkbutton.h>
#include <QtWidgets/qbuttongroup.h>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QMenu;
class QActionGroup;
class QDesignerFormWindowCursorInterface;

namespace qdesigner_internal {

// ButtonGroupMenu: Mixin menu for the 'select members'/'break group' options of
// the task menu of buttons and button group
class ButtonGroupMenu : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ButtonGroupMenu)
public:
    ButtonGroupMenu(QObject *parent = nullptr);

    void initialize(QDesignerFormWindowInterface *formWindow,
                    QButtonGroup *buttonGroup = nullptr,
                    /* Current button for selection in ButtonMode */
                    QAbstractButton *currentButton = nullptr);

    QAction *selectGroupAction() const { return m_selectGroupAction; }
    QAction *breakGroupAction() const  { return m_breakGroupAction; }

private slots:
    void selectGroup();
    void breakGroup();

private:
    QAction *m_selectGroupAction;
    QAction *m_breakGroupAction;

    QDesignerFormWindowInterface *m_formWindow = nullptr;
    QButtonGroup *m_buttonGroup = nullptr;
    QAbstractButton *m_currentButton = nullptr;
};

// Task menu extension of a QButtonGroup
class ButtonGroupTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ButtonGroupTaskMenu)
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit ButtonGroupTaskMenu(QButtonGroup *buttonGroup, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private:
    QButtonGroup *m_buttonGroup;
    QList<QAction*> m_taskActions;
    mutable ButtonGroupMenu m_menu;
};

// Task menu extension of a QAbstractButton
class ButtonTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ButtonTaskMenu)
public:
    explicit ButtonTaskMenu(QAbstractButton *button, QObject *parent = nullptr);
    ~ButtonTaskMenu() override;

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

    QAbstractButton *button() const;

protected:
    void insertAction(int index, QAction *a);

private slots:
    void createGroup();
    void addToGroup(QAction *a);
    void removeFromGroup();

private:
    enum SelectionType {
        OtherSelection,
        UngroupedButtonSelection,
        GroupedButtonSelection
    };

    SelectionType selectionType(const QDesignerFormWindowCursorInterface *cursor, QButtonGroup ** ptrToGroup = nullptr) const;
    bool refreshAssignMenu(const QDesignerFormWindowInterface *fw, int buttonCount, SelectionType st, QButtonGroup *currentGroup);
    QMenu *createGroupSelectionMenu(const QDesignerFormWindowInterface *fw);

    QList<QAction*> m_taskActions;
    mutable ButtonGroupMenu m_groupMenu;
    QMenu *m_assignGroupSubMenu;
    QActionGroup *m_assignActionGroup;
    QAction *m_assignToGroupSubMenuAction;
    QMenu *m_currentGroupSubMenu;
    QAction *m_currentGroupSubMenuAction;

    QAction *m_createGroupAction;
    QAction *m_preferredEditAction;
    QAction *m_removeFromGroupAction;
};

// Task menu extension of a QCommandLinkButton
class CommandLinkButtonTaskMenu: public ButtonTaskMenu
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CommandLinkButtonTaskMenu)
public:
    explicit CommandLinkButtonTaskMenu(QCommandLinkButton *button, QObject *parent = nullptr);
};

using ButtonGroupTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QButtonGroup, ButtonGroupTaskMenu>;
using CommandLinkButtonTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QCommandLinkButton, CommandLinkButtonTaskMenu>;
using ButtonTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QAbstractButton, ButtonTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // BUTTON_TASKMENU_H
