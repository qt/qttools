# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

my_add_library(lib3 STATIC ../lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE eo)
set(QT_I18N_TRANSLATED_LANGUAGES de)
qt_add_translations(TARGETS lib3)

my_add_library(lib4 STATIC ../lib.cpp)
set(QT_I18N_SOURCE_LANGUAGE ee)
set(QT_I18N_TRANSLATED_LANGUAGES da)
qt_add_translations(TARGETS lib4 IMMEDIATE_CALL)

