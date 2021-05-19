# Generated from windeployqt.pro.

#####################################################################
## windeployqt Tool:
#####################################################################

qt_get_tool_target_name(target_name windeployqt)
qt_internal_add_tool(${target_name}
    # BOOTSTRAP # special case remove
    TOOLS_TARGET Tools # special case
    TARGET_DESCRIPTION "Qt Windows Deployment Tool"
    SOURCES
        ../shared/winutils/elfreader.cpp ../shared/winutils/elfreader.h
        ../shared/winutils/qmlutils.cpp ../shared/winutils/qmlutils.h
        ../shared/winutils/utils.cpp ../shared/winutils/utils.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_NO_FOREACH
    INCLUDE_DIRECTORIES
        ../shared/winutils
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Core # special case
)

#### Keys ignored in scope 1:.:.:windeployqt.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Windows Deployment Tool"
# QT_FOR_CONFIG = "tools-private"
# _OPTION = "host_build"
# _REQUIREMENTS = "qtConfig(windeployqt)"

## Scopes:
#####################################################################

qt_internal_extend_target(${target_name} CONDITION WIN32
    PUBLIC_LIBRARIES
        shlwapi
)

qt_internal_extend_target(${target_name} CONDITION QT_FEATURE_relocatable
    DEFINES
        QT_RELOCATABLE
)
