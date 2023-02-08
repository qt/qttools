# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_exclude_tool_directories_from_default_target(
    lib
    components
    designer
    plugins
)

if(QT_FEATURE_process)
    add_subdirectory(lib)
    add_subdirectory(components)
    add_subdirectory(designer)
endif()
if(QT_BUILD_SHARED_LIBS AND QT_FEATURE_process)
    add_subdirectory(plugins)
endif()
