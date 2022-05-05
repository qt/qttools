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
