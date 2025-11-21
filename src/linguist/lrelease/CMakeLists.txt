# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lrelease Tool:
#####################################################################

qt_get_tool_target_name(target_name lrelease)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Compiler"
    TOOLS_TARGET Linguist
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/projectdescriptionreader.cpp ../shared/projectdescriptionreader.h
        ../shared/projsongenerator.cpp ../shared/projsongenerator.h
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/qrcreader.cpp ../shared/qrcreader.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        ../shared/parsers/xmlparser.cpp ../shared/parsers/xmlparser.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        QMakeParserCumulative
        Qt::CorePrivate
    # special case begin
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../${QT_CMAKE_EXPORT_NAMESPACE}LinguistToolsMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}LinguistToolsMacros.cmake"
    # special case end
)
qt_internal_return_unless_building_tools()
