# Generated from src.pro.

add_subdirectory(global)
add_subdirectory(linguist)
add_subdirectory(qtattributionsscanner)
if(QT_FEATURE_png AND QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton AND TARGET Qt::Widgets)
    add_subdirectory(assistant)
    add_subdirectory(designer)
    add_subdirectory(pixeltool)
endif()
if(QT_FEATURE_png AND QT_FEATURE_thread AND QT_FEATURE_toolbutton AND TARGET Qt::Quick AND TARGET Qt::Widgets)
    add_subdirectory(distancefieldgenerator)
endif()
if(QT_FEATURE_library AND (android_app OR NOT ANDROID))
    add_subdirectory(qtplugininfo)
endif()
# special case begin
qt_feature_module_begin(
    NO_MODULE
    PUBLIC_FILE "qdoc/qtqdoc-config.h"
    PRIVATE_FILE "qdoc/qtqdoc-config_p.h"
)
include(qdoc/configure.cmake)
qt_feature_module_end(NO_MODULE)
# special case end

if(QT_FEATURE_qdoc AND QT_FEATURE_thread)
    add_subdirectory(qdoc)
endif()
if(android_app OR NOT ANDROID)
    add_subdirectory(qtpaths)
endif()
if(APPLE_OSX)
    add_subdirectory(macdeployqt)
endif()
if(TARGET Qt::DBus)
    add_subdirectory(qdbus)
endif()
if(WIN32)
    add_subdirectory(windeployqt)
endif()
if(WINRT)
    add_subdirectory(winrtrunner)
endif()
if(TARGET Qt::Gui AND NOT ANDROID AND NOT APPLE_UIKIT AND NOT QNX AND NOT WASM AND NOT WINRT)
    add_subdirectory(qtdiag)
endif()
