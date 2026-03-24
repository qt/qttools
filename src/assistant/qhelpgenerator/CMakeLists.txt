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

qt_internal_add_plugin_dependencies(${target_name}
    SKIP_MISSING_PLUGIN_TARGETS
    PLUGIN_TARGETS Qt::QMinimalIntegrationPlugin Qt::QSQLiteDriverPlugin
)
