// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "abstractobjectinspector.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerObjectInspectorInterface

    \brief The QDesignerObjectInspectorInterface class allows you to
    change the focus of \QD's object inspector.

    \inmodule QtDesigner

    You can use the QDesignerObjectInspectorInterface to change the
    current form window selection. For example, when implementing a
    custom widget plugin:

    \snippet lib/tools_designer_src_lib_sdk_abstractobjectinspector.cpp 0

    The QDesignerObjectInspectorInterface class is not intended to be
    instantiated directly. You can retrieve an interface to \QD's
    object inspector using the
    QDesignerFormEditorInterface::objectInspector() function.  A
    pointer to \QD's current QDesignerFormEditorInterface object (\c
    formEditor in the example above) is provided by the
    QDesignerCustomWidgetInterface::initialize() function's
    parameter. When implementing a custom widget plugin, you must
    subclass the QDesignerCustomWidgetInterface to expose your plugin
    to \QD.

    The interface provides the core() function that you can use to
    retrieve a pointer to \QD's current QDesignerFormEditorInterface
    object, and the setFormWindow() function that enables you to
    change the current form window selection.

    \sa QDesignerFormEditorInterface, QDesignerFormWindowInterface
*/

/*!
    Constructs an object inspector interface with the given \a parent
    and the specified window \a flags.
*/
QDesignerObjectInspectorInterface::QDesignerObjectInspectorInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
    Destroys the object inspector interface.
*/
QDesignerObjectInspectorInterface::~QDesignerObjectInspectorInterface() = default;

/*!
    Returns a pointer to \QD's current QDesignerFormEditorInterface
    object.
*/
QDesignerFormEditorInterface *QDesignerObjectInspectorInterface::core() const
{
    return nullptr;
}

/*!
    \fn void QDesignerObjectInspectorInterface::setFormWindow(QDesignerFormWindowInterface *formWindow)

    Sets the currently selected form window to \a formWindow.
*/

QT_END_NAMESPACE
