# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_i18n_subdir)
find_package(Qt6 COMPONENTS Gui LinguistTools)
qt_standard_project_setup()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_subdirectory(subdir)
