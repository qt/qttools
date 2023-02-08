# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_utilities Test:
#####################################################################

qt_internal_add_test(tst_utilities
    SOURCES
        ../../../../src/qdoc/utilities.cpp ../../../../src/qdoc/utilities.h
        tst_utilities.cpp
    INCLUDE_DIRECTORIES
        ../../../../src/qdoc
)
