# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lrelease Tool:
#####################################################################

qt_get_tool_target_name(target_name lrelease)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Compiler"
    TOOLS_TARGET Linguist
    USER_FACING
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/validator.cpp ../shared/validator.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        ../shared/xmlparser.cpp ../shared/xmlparser.h
        ../shared/releasehelper.cpp ../shared/releasehelper.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        LINGUIST_CONSOLE_APPLICATION
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        LinguistProject
        Qt::CorePrivate
    # special case begin
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../${QT_CMAKE_EXPORT_NAMESPACE}LinguistToolsMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}LinguistToolsMacros.cmake"
    # special case end
)
qt_internal_return_unless_building_tools()
