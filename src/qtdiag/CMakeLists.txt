# Generated from qtdiag.pro.

#####################################################################
## qtdiag Binary:
#####################################################################

qt_add_executable(qtdiag
    SOURCES
        main.cpp
        qtdiag.cpp qtdiag.h
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
)

## Scopes:
#####################################################################

qt_extend_target(qtdiag CONDITION TARGET Qt::Widgets
    PUBLIC_LIBRARIES
        Qt::Widgets
)

qt_extend_target(qtdiag CONDITION TARGET Qt::Network
    DEFINES
        NETWORK_DIAG
    PUBLIC_LIBRARIES
        Qt::Network
)
