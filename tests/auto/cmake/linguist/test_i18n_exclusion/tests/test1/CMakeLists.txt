# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_executable(test1 test1.cpp)
target_link_libraries(test1 PRIVATE Qt6::Gui)
