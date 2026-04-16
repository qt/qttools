# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qev App:
#####################################################################

qt_internal_add_app(qev
    SOURCES
        qev.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Widgets
)
