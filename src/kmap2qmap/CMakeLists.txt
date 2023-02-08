# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

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
