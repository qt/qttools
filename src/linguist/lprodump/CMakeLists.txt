# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lprodump Tool:
#####################################################################

qt_get_tool_target_name(target_name lprodump)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Linguist QMake Project Dump Tool"
    TOOLS_TARGET Linguist
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        ../shared/projectdescriptionreader.cpp ../shared/projectdescriptionreader.h
        ../shared/projsongenerator.cpp ../shared/projsongenerator.h
        ../shared/qrcreader.cpp ../shared/qrcreader.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES

        QMakeParserCumulative
)

qt_internal_return_unless_building_tools()
