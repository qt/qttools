# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

qt_examples_build_begin(EXTERNAL_BUILD)

qt_exclude_tool_directories_from_default_target(
    assistant
)

if(TARGET Qt::Widgets)
    if(TARGET Qt::Help)
        add_subdirectory(help)
    endif()
    if(QT_FEATURE_linguist)
      add_subdirectory(linguist)
    endif()
    add_subdirectory(uitools)
endif()
if(QT_FEATURE_process AND TARGET Qt::Widgets)
    if(QT_FEATURE_designer)
        add_subdirectory(designer)
    endif()
    add_subdirectory(assistant)
endif()

qt_examples_build_end()
