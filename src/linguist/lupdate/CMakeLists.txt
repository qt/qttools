# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    message(WARNING "lupdate will not be built in this configuration.")
    return()
endif()

if (MINGW)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY _qt_skip_separate_debug_info ON)
endif()

#####################################################################
## lupdate Tool:
#####################################################################

qt_get_tool_target_name(target_name lupdate)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Update Tool"
    TOOLS_TARGET Linguist
    EXTRA_CMAKE_FILES "${CMAKE_CURRENT_LIST_DIR}/../GenerateLUpdateProject.cmake"
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/projectdescriptionreader.cpp ../shared/projectdescriptionreader.h
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/qrcreader.cpp ../shared/qrcreader.h
        ../shared/runqttool.cpp ../shared/runqttool.h
        ../shared/simtexth.cpp ../shared/simtexth.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        ../shared/xmlparser.cpp ../shared/xmlparser.h
        cpp.cpp cpp.h
        metastrings.h
        metastrings.cpp
        java.cpp
        python.cpp
        lupdate.h
        main.cpp
        merge.cpp
        ui.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::Tools
)

set_source_files_properties(python.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

qt_internal_return_unless_building_tools()

## Scopes:
#####################################################################

qt_internal_extend_target(${target_name} CONDITION TARGET Qt::QmlPrivate
    SOURCES
        qdeclarative.cpp
    LIBRARIES
        Qt::QmlPrivate
)

qt_internal_extend_target(${target_name} CONDITION NOT TARGET Qt::QmlPrivate
    DEFINES
        QT_NO_QML
)

qt_internal_extend_target(${target_name} CONDITION MSVC
    DEFINES _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
