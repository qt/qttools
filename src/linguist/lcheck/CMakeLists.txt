# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lcheck Tool:
#####################################################################

qt_get_tool_target_name(target_name lcheck)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Tool To Perform Batch Validation on a TS File"
    TOOLS_TARGET Linguist
    SOURCES
        main.cpp
        ../shared/numerus.cpp
        ../shared/validator.cpp ../shared/validator.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
    DEFINES
        LINGUIST_CONSOLE_APPLICATION
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::Tools
)

qt_internal_return_unless_building_tools()
