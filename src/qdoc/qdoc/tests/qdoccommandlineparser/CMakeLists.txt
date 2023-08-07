# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_qdoccommandlineparser Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qdoccommandlineparser LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qdoccommandlineparser
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/tst_qdoccommandlineparser.cpp

        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/qdoccommandlineparser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../../src/qdoc/utilities.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}/../../src/
)

# Resources:
set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/tst_arguments.txt"
    PROPERTIES QT_RESOURCE_ALIAS "tst_arguments.txt"
)

set(tst_qdoccommandlineparser_resource_files
    "${CMAKE_CURRENT_LIST_DIR}/tst_arguments.txt"
)

qt_internal_add_resource(tst_qdoccommandlineparser "tst_qdoccommandlineparser"
    PREFIX
        "/"
    FILES
        ${tst_qdoccommandlineparser_resource_files}
)

