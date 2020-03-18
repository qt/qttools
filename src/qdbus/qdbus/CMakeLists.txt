# Generated from qdbus.pro.

#####################################################################
## qdbus Binary:
#####################################################################

qt_add_executable(qdbus
    GUI
    SOURCES
        qdbus.cpp
    PUBLIC_LIBRARIES
        Qt::DBusPrivate
        Qt::Xml
)

## Scopes:
#####################################################################
