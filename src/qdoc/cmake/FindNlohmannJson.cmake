# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# FindNlohmannJson.cmake - Find the nlohmann/json library
#
# This module defines:
#   nlohmann_json_FOUND - System has nlohmann/json
#   nlohmann_json_INCLUDE_DIRS - The nlohmann/json include directories
#   nlohmann_json::nlohmann_json - Imported target for nlohmann/json

find_path(nlohmann_json_INCLUDE_DIR
    NAMES nlohmann/json.hpp
    PATH_SUFFIXES include
    DOC "nlohmann/json include directory"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nlohmann_json
    REQUIRED_VARS nlohmann_json_INCLUDE_DIR
    FAIL_MESSAGE "nlohmann/json not found. Install via:
    Homebrew: brew install nlohmann-json
    APT:      sudo apt install nlohmann-json3-dev
    Conan:    conan install nlohmann_json/3.11.3
    vcpkg:    vcpkg install nlohmann-json"
)

if(nlohmann_json_FOUND)
    set(nlohmann_json_INCLUDE_DIRS ${nlohmann_json_INCLUDE_DIR})

    if(NOT TARGET nlohmann_json::nlohmann_json)
        add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
        set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${nlohmann_json_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(nlohmann_json_INCLUDE_DIR)

