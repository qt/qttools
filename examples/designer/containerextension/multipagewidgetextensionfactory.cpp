// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "multipagewidgetextensionfactory.h"
#include "multipagewidgetcontainerextension.h"
#include "multipagewidget.h"

//! [0]
MultiPageWidgetExtensionFactory::MultiPageWidgetExtensionFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{}
//! [0]

//! [1]
QObject *MultiPageWidgetExtensionFactory::createExtension(QObject *object,
                                                          const QString &iid,
                                                          QObject *parent) const
{
    auto *widget = qobject_cast<MultiPageWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerContainerExtension)))
        return new MultiPageWidgetContainerExtension(widget, parent);
    return nullptr;
}
//! [1]
