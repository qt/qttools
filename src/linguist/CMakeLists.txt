# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_exclude_tool_directories_from_default_target(
    linguist
)

if(NOT QT_FEATURE_linguist)
    return()
endif()
add_subdirectory(lconvert)
add_subdirectory(lprodump)
add_subdirectory(lrelease)
add_subdirectory(lrelease-pro)
add_subdirectory(lupdate)
add_subdirectory(lupdate-pro)
if(QT_FEATURE_process AND QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton AND TARGET Qt::Widgets AND NOT no-png)
    add_subdirectory(linguist)
endif()

# Create a fake module that would emulate the Qt5::LinguistTools CMake Config package
qt_internal_add_module(Linguist NO_MODULE_HEADERS HEADER_MODULE)
