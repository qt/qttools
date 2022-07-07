# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from pixeltool.pro.

#####################################################################
## pixeltool App:
#####################################################################

qt_internal_add_app(pixeltool
    SOURCES
        main.cpp
        qpixeltool.cpp qpixeltool.h
    LIBRARIES
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

if(APPLE)
    set_target_properties(pixeltool PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info_mac.plist"
        MACOSX_BUNDLE TRUE
    )
endif()
