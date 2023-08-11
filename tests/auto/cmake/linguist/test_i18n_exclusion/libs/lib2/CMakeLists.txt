# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_library(lib2 STATIC lib2.cpp)
target_link_libraries(lib2 PRIVATE Qt6::Core)
target_include_directories(lib2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET lib2 PROPERTY QT_EXCLUDE_FROM_TRANSLATION ON)
