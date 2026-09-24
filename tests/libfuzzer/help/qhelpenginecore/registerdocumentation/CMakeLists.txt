# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(registerdocumentation LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Help Sql)

qt_add_executable(registerdocumentation
    main.cpp
)

target_link_libraries(registerdocumentation PUBLIC
    Qt::Core
    Qt::Help
    Qt::Sql
)
if(DEFINED ENV{LIB_FUZZING_ENGINE})
    target_link_libraries(registerdocumentation PRIVATE
        $ENV{LIB_FUZZING_ENGINE}
    )
else()
    target_link_libraries(registerdocumentation PRIVATE
        -fsanitize=fuzzer
    )
endif()

qt_import_plugins(registerdocumentation
    INCLUDE Qt::QSQLiteDriverPlugin
)
