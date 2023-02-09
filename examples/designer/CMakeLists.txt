# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_exclude_tool_directories_from_default_target(
    containerextension
    taskmenuextension
)

qt_internal_add_example(calculatorform)
qt_internal_add_example(calculatorform_mi)
if(QT_BUILD_SHARED_LIBS AND NOT solaris-cc_x_)
    qt_internal_add_example(calculatorbuilder)
endif()
if(QT_BUILD_SHARED_LIBS AND NOT CMAKE_CROSSCOMPILING)
    qt_internal_add_example(containerextension)
    qt_internal_add_example(customwidgetplugin)
    qt_internal_add_example(taskmenuextension)
endif()
