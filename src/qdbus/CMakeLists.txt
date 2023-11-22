# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_FEATURE_qdbus)
    return()
endif()
if(QT_FEATURE_dom)
    add_subdirectory(qdbus)
endif()
if(QT_FEATURE_dialogbuttonbox AND QT_FEATURE_inputdialog AND QT_FEATURE_menu AND QT_FEATURE_messagebox AND TARGET Qt::Widgets)
    add_subdirectory(qdbusviewer)
endif()
