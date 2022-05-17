// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LINEEDIT_TASKMENU_H
#define LINEEDIT_TASKMENU_H

#include <QtWidgets/qlineedit.h>
#include <QtCore/qpointer.h>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class LineEditTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit LineEditTaskMenu(QLineEdit *button, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private:
    QList<QAction*> m_taskActions;
    QAction *m_editTextAction;
};

using LineEditTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QLineEdit, LineEditTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LINEEDIT_TASKMENU_H
