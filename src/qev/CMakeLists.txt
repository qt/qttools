# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qev App:
#####################################################################

qt_internal_add_app(qev
    SOURCES
        qev.cpp
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Widgets
)
