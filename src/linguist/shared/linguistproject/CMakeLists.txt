# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## LinguistProject Library:
#####################################################################

qt_internal_return_unless_building_tools()

qt_internal_add_cmake_library(LinguistProject STATIC
    SOURCES
        projectdescriptionreader.cpp projectdescriptionreader.h
        projsongenerator.cpp projsongenerator.h
        qrcreader.cpp qrcreader.h
        profileutils.h
        projectprocessor.cpp projectprocessor.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII

    PUBLIC_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/..

    LIBRARIES
        TrLib
        QMakeParserCumulative
        Qt::Core
)
