// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

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
