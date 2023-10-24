# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial

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
