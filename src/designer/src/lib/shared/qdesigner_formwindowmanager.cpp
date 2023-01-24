// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_formwindowmanager_p.h"
#include "plugindialog_p.h"

#include <QtDesigner/abstractformeditor.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

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

} // namespace qdesigner_internal

QT_END_NAMESPACE
