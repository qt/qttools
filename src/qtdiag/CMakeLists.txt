# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qtdiag App:
#####################################################################

qt_internal_add_app(qtdiag
    INSTALL_VERSIONED_LINK
    SOURCES
        main.cpp
        qtdiag.cpp qtdiag.h
    LIBRARIES
        Qt::CorePrivate
        # Qt::Gui # special case QTBUG-83498
        # Qt::GuiPrivate # special case QTBUG-83498
)
set_target_properties(qtdiag PROPERTIES
    WIN32_EXECUTABLE FALSE
)

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

# QTBUG-83498
qt_internal_extend_target(qtdiag CONDITION NOT TARGET Qt::Widgets AND NOT QT_FEATURE_opengl
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::GuiPrivate # special case QTBUG-83498
)

qt_internal_extend_target(qtdiag CONDITION TARGET Qt::Network
    DEFINES
        NETWORK_DIAG
    PUBLIC_LIBRARIES
        Qt::Network
)
