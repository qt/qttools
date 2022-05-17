// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "abstractnewformwidget.h"
#include <newformwidget_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerNewFormWidgetInterface
    \since 4.5
    \internal

    \brief QDesignerNewFormWidgetInterface provides an interface for chooser
           widgets that can be used within "New Form" dialogs and wizards.
           It presents the user with a list of choices taken from built-in
           templates, pre-defined template paths and suitable custom widgets.
           It provides a static creation function that returns \QD's
           implementation.

    \inmodule QtDesigner
*/

/*!
    Constructs a QDesignerNewFormWidgetInterface object.
*/

QDesignerNewFormWidgetInterface::QDesignerNewFormWidgetInterface(QWidget *parent) :
    QWidget(parent)
{
}

/*!
    Destroys the QDesignerNewFormWidgetInterface object.
*/

QDesignerNewFormWidgetInterface::~QDesignerNewFormWidgetInterface() = default;

/*!
    Creates an instance of the QDesignerNewFormWidgetInterface as a child
    of \a parent using \a core.
*/

QDesignerNewFormWidgetInterface *QDesignerNewFormWidgetInterface::createNewFormWidget(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::NewFormWidget(core, parent);
}

/*!
    \fn bool QDesignerNewFormWidgetInterface::hasCurrentTemplate() const

    Returns whether a form template is currently selected.
*/

/*!
    \fn QString QDesignerNewFormWidgetInterface::currentTemplate(QString *errorMessage = 0)

    Returns the contents of the currently selected template. If the method fails,
    an empty string is returned and \a errorMessage receives an error message.
*/

// Signals

/*!
    \fn void QDesignerNewFormWidgetInterface::templateActivated()

    This signal is emitted whenever the user activates a template by double-clicking.
*/

/*!
    \fn void QDesignerNewFormWidgetInterface::currentTemplateChanged(bool templateSelected)

    This signal is emitted whenever the user changes the current template.
    \a templateSelected indicates whether a template is currently selected.
*/

QT_END_NAMESPACE
