# Generated from lrelease-pro.pro.

#####################################################################
## lrelease-pro Tool:
#####################################################################

qt_get_tool_target_name(target_name lrelease-pro)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Compiler for QMake Projects"
    TOOLS_TARGET Linguist # special case
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        ../shared/runqttool.cpp ../shared/runqttool.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:lrelease-pro.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Translation File Compiler for QMake Projects"
# QT_TOOL_ENV = "qmake"
# _OPTION = "host_build"
# qmake.name = "QMAKE"
# qmake.value = "$$shell_path($$QMAKE_QMAKE)"
