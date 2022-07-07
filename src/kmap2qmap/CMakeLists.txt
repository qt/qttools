# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from kmap2qmap.pro.

#####################################################################
## kmap2qmap App:
#####################################################################

qt_internal_add_app(kmap2qmap
    SOURCES
        main.cpp
    PUBLIC_LIBRARIES
        Qt::InputSupportPrivate
)
set_target_properties(kmap2qmap PROPERTIES
    WIN32_EXECUTABLE FALSE
)

#### Keys ignored in scope 1:.:.:kmap2qmap.pro:<TRUE>:
# QT_FOR_CONFIG = "tools-private"
# _REQUIREMENTS = "qtConfig(kmap2qmap)"
