# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_utilities Test:
#####################################################################

qt_internal_add_test(tst_utilities
    SOURCES
        ${QDOC_SOURCE_DIRECTORY}/utilities.cpp
        tst_utilities.cpp
    INCLUDE_DIRECTORIES
        ${QDOC_INCLUDE_DIRECTORY}
)
