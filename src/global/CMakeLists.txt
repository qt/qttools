# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from global.pro.

#####################################################################
## Tools Module:
#####################################################################

qt_internal_add_module(Tools
    INTERNAL_MODULE
    HEADER_MODULE
    # special case begin
    CONFIGURE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../configure.cmake"
    # special case end
    MODULE_INTERFACE_NAME Tools # Specify the 'Tools' name explicitly to avoid warning
                                # related to the expected name of internal module.
)

#### Keys ignored in scope 1:.:.:global.pro:<TRUE>:
# MODULE = "tools"
