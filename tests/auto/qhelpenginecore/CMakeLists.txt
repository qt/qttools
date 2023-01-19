# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_qhelpenginecore Test:
#####################################################################

qt_internal_add_test(tst_qhelpenginecore
    SOURCES
        tst_qhelpenginecore.cpp
    DEFINES
        QT_USE_USING_NAMESPACE
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARIES
        Qt::Gui
        Qt::Help
        Qt::Sql
)
