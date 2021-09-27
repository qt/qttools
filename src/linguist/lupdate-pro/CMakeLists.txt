# Generated from lupdate-pro.pro.

#####################################################################
## lupdate-pro Tool:
#####################################################################

qt_get_tool_target_name(target_name lupdate-pro)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Update Tool for QMake Projects"
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

#### Keys ignored in scope 1:.:.:lupdate-pro.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Translation File Update Tool for QMake Projects"
# QT_TOOL_ENV = "qmake"
# _OPTION = "host_build"
# qmake.name = "QMAKE"
# qmake.value = "$$shell_path($$QMAKE_QMAKE)"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:lupdate-pro.pro:MINGW:
# RC_FILE = "lupdate-pro.rc"
