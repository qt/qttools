# Generated from pixeltool.pro.

#####################################################################
## pixeltool Binary:
#####################################################################

qt_add_executable(pixeltool
    GUI
    SOURCES
        main.cpp
        qpixeltool.cpp qpixeltool.h
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Widgets
)

#### Keys ignored in scope 1:.:.:pixeltool.pro:<TRUE>:
# QT_FOR_CONFIG = "tools-private"
# _REQUIREMENTS = "qtConfig(pixeltool)"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:pixeltool.pro:APPLE:
# QMAKE_INFO_PLIST = "Info_mac.plist"
