# Generated from pixeltool.pro.

#####################################################################
## pixeltool Binary:
#####################################################################

qt_add_executable(pixeltool
    GUI
    SOURCES
        main.cpp
        qpixeltool.cpp qpixeltool.h
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Widgets
)

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:pixeltool.pro:APPLE:
# QMAKE_INFO_PLIST = "Info_mac.plist"
