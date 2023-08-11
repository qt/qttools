# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_library(lib1 STATIC lib1.cpp)
target_link_libraries(lib1 PRIVATE Qt6::Core)
target_include_directories(lib1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
