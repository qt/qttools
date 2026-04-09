# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## kmap2qmap App:
#####################################################################

qt_internal_add_app(kmap2qmap
    SOURCES
        main.cpp
    LIBRARIES
        Qt::InputSupportPrivate
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
)
set_target_properties(kmap2qmap PROPERTIES
    WIN32_EXECUTABLE FALSE
)
