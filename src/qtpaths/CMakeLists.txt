# Generated from qtpaths.pro.

#####################################################################
## qtpaths App:
#####################################################################

qt_internal_add_app(qtpaths
    SOURCES
        qtpaths.cpp
)

#### Keys ignored in scope 1:.:.:qtpaths.pro:<TRUE>:
# QT_FOR_CONFIG = "tools-private"
# _REQUIREMENTS = "qtConfig(qtpaths)"

## Scopes:
#####################################################################

if(WIN32)
    set_target_properties(qtpaths PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()
