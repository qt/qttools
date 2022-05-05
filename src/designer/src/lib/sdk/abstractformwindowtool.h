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

#ifndef ABSTRACTFORMWINDOWTOOL_H
#define ABSTRACTFORMWINDOWTOOL_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QWidget;
class QAction;
class DomUI;

class QDESIGNER_SDK_EXPORT QDesignerFormWindowToolInterface: public QObject
{
    Q_OBJECT
public:
    explicit QDesignerFormWindowToolInterface(QObject *parent = nullptr);
    virtual ~QDesignerFormWindowToolInterface();

    virtual QDesignerFormEditorInterface *core() const = 0;
    virtual QDesignerFormWindowInterface *formWindow() const = 0;
    virtual QWidget *editor() const = 0;

    virtual QAction *action() const = 0;

    virtual void activated() = 0;
    virtual void deactivated() = 0;

    virtual void saveToDom(DomUI*, QWidget*) {}
    virtual void loadFromDom(DomUI*, QWidget*) {}

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) = 0;
};

QT_END_NAMESPACE

#endif // ABSTRACTFORMWINDOWTOOL_H
