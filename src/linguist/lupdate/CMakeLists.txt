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
        ../shared/parsers/xmlparser.cpp ../shared/parsers/xmlparser.h
        ../shared/parsers/cpp.cpp ../shared/parsers/cpp.h
        ../shared/parsers/metastrings.h
        ../shared/parsers/metastrings.cpp
        ../shared/parsers/java.cpp
        ../shared/parsers/python.cpp
        ../shared/parsers/ui.cpp
        ../shared/parsers/trparser.cpp ../shared/parsers/trparser.h
        main.cpp
        merge.cpp
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
        ../shared/parsers/qdeclarative.cpp
    LIBRARIES
        Qt::QmlPrivate
)

qt_internal_extend_target(${target_name} CONDITION NOT TARGET Qt::QmlPrivate
    DEFINES
        QT_NO_QML
)
