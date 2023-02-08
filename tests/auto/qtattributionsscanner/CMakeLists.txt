# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qtattributionsscanner Test:
#####################################################################

qt_internal_add_test(tst_qtattributionsscanner
    SOURCES
        tst_qtattributionsscanner.cpp
)

target_compile_definitions(tst_qtattributionsscanner
    PRIVATE QTTOOLS_LICENSES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../../LICENSES"
)
