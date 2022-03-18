# Generated from qhelpgenerator.pro.

#####################################################################
## qhelpgenerator Tool:
#####################################################################

qt_get_tool_target_name(target_name qhelpgenerator)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Compressed Help File Generator"
    TOOLS_TARGET Tools # special case
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        ../shared/collectionconfiguration.cpp ../shared/collectionconfiguration.h
        collectionconfigreader.cpp collectionconfigreader.h
        helpgenerator.cpp helpgenerator.h
        main.cpp
        qhelpdatainterface.cpp qhelpdatainterface_p.h
        qhelpprojectdata.cpp qhelpprojectdata_p.h
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::HelpPrivate
        Qt::Network
)

if(NOT QT_BUILD_SHARED_LIBS AND QT_WILL_BUILD_TOOLS)
    if(QT_SUPERBUILD)
        # In a top-level build, qt_import_plugins() is a no-op because
        # __qt_internal_add_static_plugins_once() is not called.
        # So we need to initialize and link the plugin manually.
        set(out_file_path "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_plugin_imports_custom.cpp")

        file(GENERATE OUTPUT "${out_file_path}" CONTENT
"// This file is auto-generated. Do not edit.
#include <QtPlugin>

Q_IMPORT_PLUGIN(QMinimalIntegrationPlugin)
")

        # CMake versions earlier than 3.18.0 can't find the generated file for some reason,
        # failing at generation phase.
        # Explicitly marking the file as GENERATED fixes the issue.
        set_source_files_properties("${out_file_path}" PROPERTIES GENERATED TRUE)

        target_sources(${target_name} PRIVATE "${out_file_path}")
        target_link_libraries(${target_name} PRIVATE Qt::QMinimalIntegrationPlugin)
    else()
        qt_import_plugins(${target_name}
            INCLUDE Qt::QMinimalIntegrationPlugin
        )
    endif()
endif()
