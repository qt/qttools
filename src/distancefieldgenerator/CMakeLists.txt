# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

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

qt_internal_add_docs(qdistancefieldgenerator
    doc/qtdistancefieldgenerator.qdocconf
)

