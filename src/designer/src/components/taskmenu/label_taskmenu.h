// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LABEL_TASKMENU_H
#define LABEL_TASKMENU_H

#include <QtWidgets/qlabel.h>
#include <QtCore/qpointer.h>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class LabelTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit LabelTaskMenu(QLabel *button, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private slots:
    void editRichText();

private:
    QLabel *m_label;
    QList<QAction*> m_taskActions;
    QAction *m_editRichTextAction;
    QAction *m_editPlainTextAction;
};

using LabelTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QLabel, LabelTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LABEL_TASKMENU_H
