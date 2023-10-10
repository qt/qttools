# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qhelpgenerator Tool:
#####################################################################

qt_get_tool_target_name(target_name qhelpgenerator)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Compressed Help File Generator"
    TOOLS_TARGET Tools
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        ../shared/collectionconfiguration.cpp ../shared/collectionconfiguration.h
        collectionconfigreader.cpp collectionconfigreader.h
        helpgenerator.cpp helpgenerator.h
        main.cpp
        qhelpdatainterface.cpp qhelpdatainterface_p.h
        qhelpprojectdata.cpp qhelpprojectdata_p.h
    LIBRARIES
        Qt::Gui
        Qt::HelpPrivate
)
qt_internal_return_unless_building_tools()

set(needed_plugins
    QMinimalIntegrationPlugin
    QSQLiteDriverPlugin
)
list(TRANSFORM needed_plugins PREPEND Qt:: OUTPUT_VARIABLE needed_plugin_targets)

if(NOT QT_BUILD_SHARED_LIBS)
    if(QT_SUPERBUILD)
        # In a top-level build, qt_import_plugins() is a no-op because
        # __qt_internal_add_static_plugins_once() is not called.
        # So we need to initialize and link the plugin manually.
        set(out_file_path "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_plugin_imports_custom.cpp")

        # Create a string with the necessary Q_IMPORT_PLUGIN(...) statements.
        list(TRANSFORM needed_plugins PREPEND "Q_IMPORT_PLUGIN(" OUTPUT_VARIABLE import_plugin_code)
        list(TRANSFORM import_plugin_code APPEND ")")
        list(JOIN import_plugin_code "\n" import_plugin_code)

        file(GENERATE OUTPUT "${out_file_path}" CONTENT
"// This file is auto-generated. Do not edit.
#include <QtPlugin>

${import_plugin_code}
")

        # CMake versions earlier than 3.18.0 can't find the generated file for some reason,
        # failing at generation phase.
        # Explicitly marking the file as GENERATED fixes the issue.
        set_source_files_properties("${out_file_path}" PROPERTIES GENERATED TRUE)

        target_sources(${target_name} PRIVATE "${out_file_path}")
        target_link_libraries(${target_name} PRIVATE ${needed_plugin_targets})
    else()
        qt_import_plugins(${target_name}
            INCLUDE ${needed_plugin_targets}
        )
    endif()
else() # QT_BUILD_SHARED_LIBS
    if(TARGET ${target_name} AND QT_SUPERBUILD)
        add_dependencies(${target_name} ${needed_plugins})
    endif()
endif()
