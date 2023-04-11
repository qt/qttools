# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Need to stop building these apps by default because they would fail
# in device_and_simulator and webassembly builds.
if(IOS OR WASM)
   set(_qt_additional_tools_to_exclude qtplugininfo)
endif()

qt_exclude_tool_directories_from_default_target(
    distancefieldgenerator
    pixeltool
    ${_qt_additional_tools_to_exclude}
)

# Evaluate features to decide what to build.
# The config files will be written in the src/global module.
qt_feature_evaluate_features("${CMAKE_CURRENT_SOURCE_DIR}/../configure.cmake")

if(TARGET Qt::Widgets)
    add_subdirectory(uiplugin)
    add_subdirectory(uitools)
endif()

add_subdirectory(global) # special case add as first directory
if(QT_FEATURE_linguist)
    add_subdirectory(linguist)
endif()
# add_subdirectory(global) # special case remove
if(QT_FEATURE_designer)
    add_subdirectory(designer)
endif()
if(QT_FEATURE_pixeltool)
    add_subdirectory(pixeltool)
endif()
if(QT_FEATURE_assistant)
    add_subdirectory(assistant)
endif()
if(QT_FEATURE_distancefieldgenerator)
    add_subdirectory(distancefieldgenerator)
endif()
if(QT_FEATURE_qtattributionsscanner)
    add_subdirectory(qtattributionsscanner)
endif()
if(QT_FEATURE_qtplugininfo)
    add_subdirectory(qtplugininfo)
endif()

add_subdirectory(qdoc)

if(QT_FEATURE_qdbus)
    add_subdirectory(qdbus)
endif()
if(QT_FEATURE_qtdiag)
    add_subdirectory(qtdiag)
endif()
