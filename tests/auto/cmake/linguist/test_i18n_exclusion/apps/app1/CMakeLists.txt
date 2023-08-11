# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_executable(app1 main.cpp)
target_link_libraries(app1 PRIVATE
    Qt6::Gui
    lib1
    lib2
)
