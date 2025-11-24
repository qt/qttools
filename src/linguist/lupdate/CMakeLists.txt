# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lupdate Tool:
#####################################################################

qt_get_tool_target_name(target_name lupdate)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Update Tool"
    TOOLS_TARGET Linguist
    EXTRA_CMAKE_FILES "${CMAKE_CURRENT_LIST_DIR}/../GenerateLUpdateProject.cmake"
    SOURCES
        ../shared/po.cpp
        ../shared/projectdescriptionreader.cpp ../shared/projectdescriptionreader.h
        ../shared/projsongenerator.cpp ../shared/projsongenerator.h
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/qrcreader.cpp ../shared/qrcreader.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        TrLib
        QMakeParserCumulative
        Qt::Tools
)
