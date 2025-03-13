# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(test_i18n_subdir2)
find_package(Qt6 COMPONENTS Gui LinguistTools)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES fr)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_subdirectory(subdir)
