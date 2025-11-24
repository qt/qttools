# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## TrLib Library:
#####################################################################

qt_internal_return_unless_building_tools()

qt_internal_add_cmake_library(TrLib STATIC
    SOURCES
        ../translator.cpp ../translator.h
        ../translatormessage.cpp ../translatormessage.h
        trparser.cpp trparser.h
        merge.cpp
        cpp.cpp cpp.h
        python.cpp
        java.cpp
        ui.cpp
        metastrings.cpp metastrings.h
        ../numerus.cpp
        ../simtexth.cpp ../simtexth.h
        ../xmlparser.cpp ../xmlparser.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    PUBLIC_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/..

    LIBRARIES
        Qt::CorePrivate
        Qt::Tools
)

set_source_files_properties(python.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

## Scopes:
#####################################################################

qt_internal_extend_target(TrLib CONDITION TARGET Qt::QmlPrivate
    SOURCES
        qdeclarative.cpp
    LIBRARIES
        Qt::QmlPrivate
)

qt_internal_extend_target(TrLib CONDITION NOT TARGET Qt::QmlPrivate
    DEFINES
        QT_NO_QML
)
