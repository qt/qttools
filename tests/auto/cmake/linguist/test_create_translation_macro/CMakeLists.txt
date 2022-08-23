# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_create_translation_macros)

find_package(Qt6 COMPONENTS Core LinguistTools REQUIRED)

set(my_srcs myi18nobject.cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/some_dir")

qt_create_translation(qm_files ${my_srcs} myobject_de.ts OPTIONS -pro-debug) # play around with OPTIONS

add_executable(myobject ${my_srcs} ${qm_files})
target_link_libraries(myobject Qt::Core)
