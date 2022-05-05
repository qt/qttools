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
