# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qtplugininfo App:
#####################################################################

qt_internal_add_app(qtplugininfo
    SOURCES
        qtplugininfo.cpp
)
set_target_properties(qtplugininfo PROPERTIES
    WIN32_EXECUTABLE FALSE
)
