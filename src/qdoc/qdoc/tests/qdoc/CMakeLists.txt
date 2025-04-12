# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_QDoc LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_QDoc
  SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp

    ${CMAKE_CURRENT_LIST_DIR}/boundaries/filesystem/catch_filepath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/boundaries/filesystem/catch_directorypath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/filesystem/catch_fileresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node/catch_genustypes.cpp

    ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/boundaries/filesystem/filepath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/boundaries/filesystem/directorypath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/boundaries/filesystem/resolvedfile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/filesystem/fileresolver.cpp

    ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/genustypes.h
  INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_LIST_DIR}/../../src/
  LIBRARIES
    Qt::QDocCatchPrivate
    Qt::QDocCatchConversionsPrivate
    Qt::QDocCatchGeneratorsPrivate
)
