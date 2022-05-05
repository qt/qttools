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

#include "qdesigner_objectinspector_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QDesignerObjectInspector::QDesignerObjectInspector(QWidget *parent, Qt::WindowFlags flags) :
    QDesignerObjectInspectorInterface(parent, flags)
{
}

void QDesignerObjectInspector::mainContainerChanged()
{
}

void Selection::clear()
{
    managed.clear();
    unmanaged.clear();
    objects.clear();
}

bool Selection::empty() const
{
    return managed.isEmpty() && unmanaged.isEmpty() && objects.isEmpty();
}

QObjectList Selection::selection() const
{
    QObjectList rc(objects);
    for (QObject *o : managed)
        rc.push_back(o);
    for (QObject *o : unmanaged)
        rc.push_back(o);
    return rc;
}
}

QT_END_NAMESPACE
