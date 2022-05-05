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

#include "qdesigner_formwindowmanager_p.h"
#include "plugindialog_p.h"

#include <QtDesigner/abstractformeditor.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

/*!
    \class qdesigner_internal::QDesignerFormWindowManager
    \inmodule QtDesigner
    \internal

    Extends QDesignerFormWindowManagerInterface with methods to control
    the preview and printing of forms. It provides a facade that simplifies
    the complexity of the more general PreviewConfiguration & PreviewManager
    interfaces.

    \since 4.5
  */


QDesignerFormWindowManager::QDesignerFormWindowManager(QObject *parent)
    : QDesignerFormWindowManagerInterface(parent)
{
}

QDesignerFormWindowManager::~QDesignerFormWindowManager() = default;

/*!
    \fn PreviewManager *qdesigner_internal::QDesignerFormWindowManager::previewManager() const

    Accesses the previewmanager implementation.

    \since 4.5
    \internal
  */

void QDesignerFormWindowManager::showPluginDialog()
{
    PluginDialog dlg(core(), core()->topLevel());
    dlg.exec();
}

QT_END_NAMESPACE
