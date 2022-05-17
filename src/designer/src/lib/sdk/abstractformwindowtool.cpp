// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "abstractformwindowtool.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerFormWindowToolInterface

    \brief The QDesignerFormWindowToolInterface class provides an
    interface that enables tools to be used on items in a form window.

    \inmodule QtDesigner

    \internal
*/

/*!
*/
QDesignerFormWindowToolInterface::QDesignerFormWindowToolInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
*/
QDesignerFormWindowToolInterface::~QDesignerFormWindowToolInterface() = default;

/*!
    \fn virtual QDesignerFormEditorInterface *QDesignerFormWindowToolInterface::core() const = 0
*/

/*!
    \fn virtual QDesignerFormWindowInterface *QDesignerFormWindowToolInterface::formWindow() const = 0
*/

/*!
    \fn virtual QWidget *QDesignerFormWindowToolInterface::editor() const = 0
*/

/*!
    \fn virtual QAction *QDesignerFormWindowToolInterface::action() const = 0
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::activated() = 0
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::deactivated() = 0
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::saveToDom(DomUI*, QWidget*) {
*/

/*!
    \fn virtual void QDesignerFormWindowToolInterface::loadFromDom(DomUI*, QWidget*) {
*/

/*!
    \fn virtual bool QDesignerFormWindowToolInterface::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) = 0
*/

QT_END_NAMESPACE
