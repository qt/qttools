# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qdbus App:
#####################################################################

qt_internal_add_app(qdbus
    SOURCES
        qdbus.cpp
    LIBRARIES
        Qt::DBusPrivate
        Qt::Xml
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
)

## Scopes:
#####################################################################

if(WIN32)
    set_target_properties(qdbus PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()
