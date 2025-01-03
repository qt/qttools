// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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

using GroupBoxTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QGroupBox, GroupBoxTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // GROUPBOX_TASKMENU_H
