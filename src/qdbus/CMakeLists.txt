# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qdbus.pro.

if(NOT QT_FEATURE_qdbus)
    return()
endif()
if(QT_FEATURE_dom)
    add_subdirectory(qdbus)
endif()
if(QT_FEATURE_dialogbuttonbox AND QT_FEATURE_inputdialog AND QT_FEATURE_menu AND QT_FEATURE_messagebox AND TARGET Qt::Widgets)
    add_subdirectory(qdbusviewer)
endif()
