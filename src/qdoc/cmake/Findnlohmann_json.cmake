# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Findnlohmann_json.cmake - Find the nlohmann/json library
#
# This module defines:
#   nlohmann_json_FOUND - System has nlohmann/json
#   nlohmann_json_INCLUDE_DIR - The nlohmann/json include directory (single)
#   nlohmann_json_INCLUDE_DIRS - The nlohmann/json include directories (list)
#   nlohmann_json::nlohmann_json - Imported target for nlohmann/json
#
# The module first tries pkg-config, then falls back to direct path search.

# Try pkg-config first
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_nlohmann_json QUIET nlohmann_json nlohmann-json)
endif()

# Use pkg-config results if available, otherwise search directly
if(PC_nlohmann_json_FOUND)
    # pkg-config found it - extract include dirs
    set(nlohmann_json_INCLUDE_DIRS ${PC_nlohmann_json_INCLUDE_DIRS})
    # FPHSA expects a single directory, so extract the first one (if any)
    if(nlohmann_json_INCLUDE_DIRS)
        list(GET nlohmann_json_INCLUDE_DIRS 0 nlohmann_json_INCLUDE_DIR)
    endif()
else()
    find_path(nlohmann_json_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        HINTS
            # Homebrew (Apple Silicon)
            /opt/homebrew/opt/nlohmann-json
            # Homebrew (Intel)
            /usr/local/opt/nlohmann-json
            # Environment variable override
            ENV NLOHMANN_JSON_ROOT
        PATH_SUFFIXES include
        DOC "nlohmann/json include directory"
    )
    # Set INCLUDE_DIRS from the single directory for consistency
    if(nlohmann_json_INCLUDE_DIR)
        set(nlohmann_json_INCLUDE_DIRS ${nlohmann_json_INCLUDE_DIR})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nlohmann_json
    REQUIRED_VARS nlohmann_json_INCLUDE_DIR
    FAIL_MESSAGE "nlohmann/json not found. Install via:
    Homebrew: brew install nlohmann-json
    APT:      sudo apt install nlohmann-json3-dev
    Conan:    conan install nlohmann_json/3.11.3
    vcpkg:    vcpkg install nlohmann-json"
)

if(nlohmann_json_FOUND AND NOT TARGET nlohmann_json::nlohmann_json)
    add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
    set_property(TARGET nlohmann_json::nlohmann_json PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${nlohmann_json_INCLUDE_DIRS}
    )
endif()

mark_as_advanced(nlohmann_json_INCLUDE_DIR nlohmann_json_INCLUDE_DIRS)

