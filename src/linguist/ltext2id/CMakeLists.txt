# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## ltext2id Tool:
#####################################################################

qt_get_tool_target_name(target_name ltext2id)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Migration Tool From Text-Based to Id-Based Translation"
    TOOLS_TARGET Linguist
    USER_FACING
    SOURCES
        main.cpp
        utils.h utils.cpp
        filetransformer.h filetransformer.cpp
        fileverifier.h fileverifier.cpp
        ../shared/xmlparser.cpp ../shared/xmlparser.h
        ../shared/ts.cpp
        recorddirectory.h recorddirectory.cpp
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        TrLib
        Qt::CorePrivate
        Qt::Tools
)
