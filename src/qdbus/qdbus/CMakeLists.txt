# Generated from qdbus.pro.

#####################################################################
## qdbus App:
#####################################################################

qt_internal_add_app(qdbus
    SOURCES
        qdbus.cpp
    PUBLIC_LIBRARIES
        Qt::DBusPrivate
        Qt::Xml
)

## Scopes:
#####################################################################

if(WIN32)
    set_target_properties(qdbus PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()
