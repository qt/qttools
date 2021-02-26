# Generated from qtdiag.pro.

#####################################################################
## qtdiag App:
#####################################################################

qt_internal_add_app(qtdiag
    INSTALL_VERSIONED_LINK
    SOURCES
        main.cpp
        qtdiag.cpp qtdiag.h
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        # Qt::Gui # special case QTBUG-83498
        # Qt::GuiPrivate # special case QTBUG-83498
)
set_target_properties(qtdiag PROPERTIES
    WIN32_EXECUTABLE FALSE
)

#### Keys ignored in scope 1:.:.:qtdiag.pro:<TRUE>:
# QT_FOR_CONFIG = "tools-private"
# _REQUIREMENTS = "qtConfig(qtdiag)"

## Scopes:
#####################################################################

qt_internal_extend_target(qtdiag CONDITION QT_FEATURE_opengl
    PUBLIC_LIBRARIES
        Qt::OpenGL
        Qt::Gui # special case QTBUG-83498
        Qt::GuiPrivate # special case QTBUG-83498
)

qt_internal_extend_target(qtdiag CONDITION TARGET Qt::Widgets
    PUBLIC_LIBRARIES
        Qt::Widgets
        Qt::Gui # special case QTBUG-83498
        Qt::GuiPrivate # special case QTBUG-83498
)

# special case begin
# QTBUG-83498
qt_internal_extend_target(qtdiag CONDITION NOT TARGET Qt::Widgets AND NOT QT_FEATURE_opengl
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::GuiPrivate # special case QTBUG-83498
)
# special case end

qt_internal_extend_target(qtdiag CONDITION TARGET Qt::Network
    DEFINES
        NETWORK_DIAG
    PUBLIC_LIBRARIES
        Qt::Network
)
