# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from distancefieldgenerator.pro.

#####################################################################
## qdistancefieldgenerator App:
#####################################################################

qt_internal_add_app(qdistancefieldgenerator
    SOURCES
        distancefieldmodel.cpp distancefieldmodel.h
        distancefieldmodelworker.cpp distancefieldmodelworker.h
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::QuickPrivate
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)
set_target_properties(qdistancefieldgenerator PROPERTIES
    QT_TARGET_DESCRIPTION "Qt Distance Field Generator"
)

#### Keys ignored in scope 1:.:.:distancefieldgenerator.pro:<TRUE>:
# QT_FOR_CONFIG = "tools-private"
# _REQUIREMENTS = "qtConfig(distancefieldgenerator)"
qt_internal_add_docs(qdistancefieldgenerator
    doc/qtdistancefieldgenerator.qdocconf
)

