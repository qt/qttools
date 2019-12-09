# Generated from windeployqt.pro.

#####################################################################
## windeployqt Tool:
#####################################################################

qt_add_tool(windeployqt
    #BOOTSTRAP # special case
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
    LIBRARIES
        Qt::CorePrivate
        Qt::Core # special case
)

#### Keys ignored in scope 1:.:.:windeployqt.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Windows Deployment Tool"
# _OPTION = "host_build"

## Scopes:
#####################################################################

qt_extend_target(windeployqt CONDITION WIN32
    PUBLIC_LIBRARIES
        shlwapi
)

qt_extend_target(windeployqt CONDITION QT_FEATURE_relocatable
    DEFINES
        QT_RELOCATABLE
)
