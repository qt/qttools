# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_QDoc LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_QDoc
    SOURCES
        main.cpp
        boundaries/filesystem/catch_filepath.cpp
        boundaries/filesystem/catch_directorypath.cpp
        filesystem/catch_fileresolver.cpp
        node/catch_genustypes.cpp
        inclusion/catch_inclusionstrategytests.cpp
        ../../src/qdoc/boundaries/filesystem/filepath.cpp
        ../../src/qdoc/boundaries/filesystem/directorypath.cpp
        ../../src/qdoc/boundaries/filesystem/resolvedfile.cpp
        ../../src/qdoc/filesystem/fileresolver.cpp
        ../../src/qdoc/genustypes.h
    INCLUDE_DIRECTORIES
        ../../src/
    LIBRARIES
        Qt::QDocCatchPrivate
        Qt::QDocCatchConversionsPrivate
        Qt::QDocCatchGeneratorsPrivate
)

qt_internal_extend_target(tst_QDoc
    CONDITION QT_QDOC_ENABLE_TEMPLATE_GENERATOR
    SOURCES
        templategenerator/catch_templategenerator.cpp
)
