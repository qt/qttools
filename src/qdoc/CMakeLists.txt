# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(catch)
add_subdirectory(catch_conversions)
add_subdirectory(catch_generators)

if(QT_FEATURE_qdoc)
    add_subdirectory(qdoc)
endif()
