# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(test_create_translation_same_base_names)

find_package(Qt6 COMPONENTS Core LinguistTools REQUIRED)

set(my_srcs myi18nobject.cpp)

set(ts_files "")
foreach(language de fr)
    set(ts_file ${language}/myobject.ts)
    set_property(SOURCE ${ts_file}
        PROPERTY OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${language})
    list(APPEND ts_files ${ts_file})
endforeach()
qt_create_translation(qm_files ${my_srcs} ${ts_files})

add_executable(myobject ${my_srcs} ${qm_files})
target_link_libraries(myobject Qt::Core)
