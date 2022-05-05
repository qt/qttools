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

#ifndef GROUPBOX_TASKMENU_H
#define GROUPBOX_TASKMENU_H

#include <QtWidgets/qgroupbox.h>
#include <QtCore/qpointer.h>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
class InPlaceEditor;

class GroupBoxTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private:
    QAction *m_editTitleAction;
    QList<QAction*> m_taskActions;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QGroupBox, GroupBoxTaskMenu>  GroupBoxTaskMenuFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // GROUPBOX_TASKMENU_H
