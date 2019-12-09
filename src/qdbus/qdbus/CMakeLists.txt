# Generated from qdbus.pro.

#####################################################################
## qdbus Binary:
#####################################################################

qt_add_executable(qdbus
    GUI
    SOURCES
        qdbus.cpp
    LIBRARIES
        Qt::DBusPrivate
    PUBLIC_LIBRARIES
        Qt::DBus
        Qt::Xml
)

## Scopes:
#####################################################################
