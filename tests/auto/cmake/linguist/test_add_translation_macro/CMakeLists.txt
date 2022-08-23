# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_add_translation_macro)

find_package(Qt6 COMPONENTS Core LinguistTools REQUIRED)

set(my_srcs myi18nobject.cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/some_dir")

qt_add_translation(qm_files myobject_de.ts OPTIONS -compress) # play around with OPTIONS

add_executable(test_add_translation ${my_srcs} ${qm_files})
target_link_libraries(test_add_translation Qt::Core)
