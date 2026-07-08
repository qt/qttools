// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only

// Emulates an input source file passed to CMake, with generator expressions that
// makes it syntactically invalid C++.

#include <QtCore/qtsymbolmacros.h>
#include <QtQml/qqmlextensionplugin.h>

$<$<BOOL:qml_register_types_QtFooBar>:QT_DECLARE_EXTERN_SYMBOL_VOID($<JOIN:qml_register_types_QtFooBar,)
QT_DECLARE_EXTERN_SYMBOL_VOID(>)>

class QtFooBarPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
    QtFooBarPlugin(QObject *parent = nullptr) : QQmlEngineExtensionPlugin(parent)
    {

$<$<BOOL:qml_register_types_QtFooBar>:QT_KEEP_SYMBOL($<JOIN:qml_register_types_QtFooBar,)
QT_KEEP_SYMBOL(>)>
    }
};

#include "FooBarQuickplugin_QtFooBarPlugin.moc"
