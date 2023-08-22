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
        ../shared/ioutils.cpp ../shared/ioutils.h
        ../shared/profileevaluator.cpp ../shared/profileevaluator.h
        ../shared/proitems.cpp ../shared/proitems.h
        ../shared/qmake_global.h
        ../shared/qmakebuiltins.cpp
        ../shared/qmakeevaluator.cpp ../shared/qmakeevaluator.h ../shared/qmakeevaluator_p.h
        ../shared/qmakeglobals.cpp ../shared/qmakeglobals.h
        ../shared/qmakeparser.cpp ../shared/qmakeparser.h
        ../shared/qmakevfs.cpp ../shared/qmakevfs.h
        ../shared/qrcreader.cpp ../shared/qrcreader.h
        main.cpp
    DEFINES
        PROEVALUATOR_CUMULATIVE
        PROEVALUATOR_DEBUG
        PROEVALUATOR_INIT_PROPS
        QMAKE_BUILTIN_PRFS
        QMAKE_OVERRIDE_PRFS
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
)

set_source_files_properties(../shared/qmakeparser.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

qt_internal_return_unless_building_tools()

# Resources:
set(proparser_resource_files
    "../shared/exclusive_builds.prf"
)

qt_internal_add_resource(${target_name} "proparser"
    PREFIX
        "/qmake/override_features"
    BASE
        "../shared"
    FILES
        ${proparser_resource_files}
)

qt_internal_extend_target(${target_name} CONDITION WIN32
    SOURCES
        ../shared/registry.cpp
        ../shared/registry_p.h
    DEFINES
        _SCL_SECURE_NO_WARNINGS
)
