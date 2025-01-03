// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
