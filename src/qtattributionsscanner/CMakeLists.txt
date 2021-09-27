# Generated from qtattributionsscanner.pro.

#####################################################################
## qtattributionsscanner Tool:
#####################################################################

qt_get_tool_target_name(target_name qtattributionsscanner)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Source Code Attribution Scanner"
    TOOLS_TARGET Tools # special case
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        jsongenerator.cpp jsongenerator.h
        logging.h
        main.cpp
        package.h
        packagefilter.cpp packagefilter.h
        qdocgenerator.cpp qdocgenerator.h
        scanner.cpp scanner.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:qtattributionsscanner.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Source Code Attribution Scanner"
# QT_FOR_CONFIG = "tools-private"
# _OPTION = "host_build"
# _REQUIREMENTS = "qtConfig(qtattributionsscanner)"
