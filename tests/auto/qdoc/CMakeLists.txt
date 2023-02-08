# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND QT_FEATURE_debug_and_release)
    message(WARNING "qdoc tests will not be built in this configuration.")
    return()
endif()

add_subdirectory(catch_generators)
add_subdirectory(qdoc)
add_subdirectory(config)
add_subdirectory(generatedoutput)
add_subdirectory(qdoccommandlineparser)
add_subdirectory(utilities)
