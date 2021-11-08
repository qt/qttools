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
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:qhelpgenerator.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Compressed Help File Generator"
# QTPLUGIN.platforms = "qminimal"
# QTPLUGIN.sqldrivers = "qsqlite"
