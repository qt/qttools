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

#ifndef TEXTEDIT_TASKMENU_H
#define TEXTEDIT_TASKMENU_H

#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qplaintextedit.h>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TextEditTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit TextEditTaskMenu(QTextEdit *button, QObject *parent = nullptr);
    explicit TextEditTaskMenu(QPlainTextEdit *button, QObject *parent = nullptr);

    ~TextEditTaskMenu() override;

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private slots:
    void editText();

private:
    void initialize();

    const Qt::TextFormat m_format;
    const QString m_property;
    const QString m_windowTitle;

    mutable QList<QAction*> m_taskActions;
    QAction *m_editTextAction;
};

using TextEditTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QTextEdit, TextEditTaskMenu>;
using PlainTextEditTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QPlainTextEdit, TextEditTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TEXTEDIT_TASKMENU_H
