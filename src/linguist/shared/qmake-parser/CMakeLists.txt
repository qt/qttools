# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QMakeParserCumulative Library:
#####################################################################

qt_internal_return_unless_building_tools()

qt_internal_add_cmake_library(QMakeParserCumulative STATIC
    SOURCES
        ioutils.cpp ioutils.h
        profileevaluator.cpp profileevaluator.h
        proitems.cpp proitems.h
        qmake_global.h
        qmakebuiltins.cpp
        qmakeevaluator.cpp qmakeevaluator.h qmakeevaluator_p.h
        qmakeglobals.cpp qmakeglobals.h
        qmakeparser.cpp qmakeparser.h
        qmakevfs.cpp qmakevfs.h
    NO_PCH_SOURCES
        qmakeevaluator.cpp
        qmakeparser.cpp
    PUBLIC_DEFINES
        PROEVALUATOR_CUMULATIVE
        PROEVALUATOR_DEBUG
        PROEVALUATOR_INIT_PROPS
    DEFINES
        QMAKE_BUILTIN_PRFS
        QMAKE_OVERRIDE_PRFS
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_USE_QSTRINGBUILDER
    PUBLIC_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES
        Qt::CorePrivate
)

qt_internal_add_resource(QMakeParserCumulative "proparser"
    PREFIX
        "/qmake/override_features"
    BASE
        ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        exclusive_builds.prf
)

qt_internal_extend_target(QMakeParserCumulative CONDITION WIN32
    SOURCES
        registry.cpp
        registry_p.h
    DEFINES
        _SCL_SECURE_NO_WARNINGS
)
