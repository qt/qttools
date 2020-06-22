# Generated from winrtrunner.pro.

#####################################################################
## winrtrunner Tool:
#####################################################################

qt_get_tool_target_name(target_name winrtrunner)
qt_add_tool(${target_name}
    BOOTSTRAP
    TARGET_DESCRIPTION "Qt WinRT Runner"
    SOURCES
        ../shared/corecon/ccapi.h
        ../shared/corecon/ccapi_11.h
        ../shared/corecon/ccapi_12.h
        ../shared/corecon/corecon.cpp ../shared/corecon/corecon.h
        ../shared/winutils/elfreader.cpp ../shared/winutils/elfreader.h
        ../shared/winutils/qmlutils.cpp ../shared/winutils/qmlutils.h
        ../shared/winutils/utils.cpp ../shared/winutils/utils.h
        appxengine.cpp appxengine.h appxengine_p.h
        appxlocalengine.cpp appxlocalengine.h
        appxphoneengine.cpp appxphoneengine.h
        main.cpp
        runner.cpp runner.h
        runnerengine.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        WINRT_LIBRARY
    INCLUDE_DIRECTORIES
        ../shared/corecon
        ../shared/winutils
    PUBLIC_LIBRARIES
        Shlwapi
        crypt32
        runtimeobject
        urlmon
        wsclient
        xmllite
)

#### Keys ignored in scope 1:.:.:winrtrunner.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt WinRT Runner"
# QT_FOR_CONFIG = "tools-private"
# _OPTION = "host_build"
# _REQUIREMENTS = "qtConfig(winrtrunner)"
