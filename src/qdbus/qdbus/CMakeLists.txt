# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qdbus.pro.

#####################################################################
## qdbus App:
#####################################################################

qt_internal_add_app(qdbus
    SOURCES
        qdbus.cpp
    LIBRARIES
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
