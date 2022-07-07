# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qhelpprojectdata.pro.

#####################################################################
## tst_qhelpprojectdata Test:
#####################################################################

qt_internal_add_test(tst_qhelpprojectdata
    SOURCES
        ../../../src/assistant/qhelpgenerator/qhelpdatainterface.cpp ../../../src/assistant/qhelpgenerator/qhelpdatainterface_p.h
        ../../../src/assistant/qhelpgenerator/qhelpprojectdata.cpp ../../../src/assistant/qhelpgenerator/qhelpprojectdata_p.h
        tst_qhelpprojectdata.cpp
    DEFINES
        QT_USE_USING_NAMESPACE
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"
    LIBRARIES
        Qt::Gui
        Qt::HelpPrivate
)
