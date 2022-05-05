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

#ifndef CONTAINER_H
#define CONTAINER_H

#include <QtDesigner/extension.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QWidget;

class QDesignerContainerExtension
{
public:
    Q_DISABLE_COPY_MOVE(QDesignerContainerExtension)

    QDesignerContainerExtension() = default;
    virtual ~QDesignerContainerExtension() = default;

    virtual int count() const = 0;
    virtual QWidget *widget(int index) const = 0;

    virtual int currentIndex() const = 0;
    virtual void setCurrentIndex(int index) = 0;

    virtual bool canAddWidget() const = 0;
    virtual void addWidget(QWidget *widget) = 0;
    virtual void insertWidget(int index, QWidget *widget) = 0;
    virtual bool canRemove(int index) const = 0;
    virtual void remove(int index) = 0;
};

Q_DECLARE_EXTENSION_INTERFACE(QDesignerContainerExtension, "org.qt-project.Qt.Designer.Container")

QT_END_NAMESPACE

#endif // CONTAINER_H
