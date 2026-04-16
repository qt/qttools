# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QHelpEnginePlugin
    OUTPUT_NAME helpplugin
    PLUGIN_TYPE help
    SOURCES
        qhelpengineplugin.h
        qhelpengineplugin.cpp
    LIBRARIES
        Qt::Core
        Qt::QmlLSPrivate
        Qt::Help
)
