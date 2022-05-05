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

#ifndef ABSTRACTDNDITEM_H
#define ABSTRACTDNDITEM_H

#include <QtDesigner/sdk_global.h>

QT_BEGIN_NAMESPACE

class DomUI;
class QWidget;
class QPoint;

class QDESIGNER_SDK_EXPORT QDesignerDnDItemInterface
{
public:
    Q_DISABLE_COPY_MOVE(QDesignerDnDItemInterface)

    enum DropType { MoveDrop, CopyDrop };

    QDesignerDnDItemInterface() = default;
    virtual ~QDesignerDnDItemInterface() = default;

    virtual DomUI *domUi() const = 0;
    virtual QWidget *widget() const = 0;
    virtual QWidget *decoration() const = 0;
    virtual QPoint hotSpot() const = 0;
    virtual DropType type() const = 0;
    virtual QWidget *source() const = 0;
};

QT_END_NAMESPACE

#endif // ABSTRACTDNDITEM_H
