# Generated from windeployqt.pro.

#####################################################################
## windeployqt Tool:
#####################################################################

qt_internal_add_executable(windeployqt # special case
    #BOOTSTRAP # special case remove
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

qt_internal_extend_target(windeployqt CONDITION WIN32 # special case
    PUBLIC_LIBRARIES
        shlwapi
)

qt_internal_extend_target(windeployqt CONDITION QT_FEATURE_relocatable # special case
    DEFINES
        QT_RELOCATABLE
)
