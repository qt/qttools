# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# FindInja.cmake - Find the Inja template engine library
#
# This module defines:
#   Inja_FOUND - System has Inja
#   Inja_INCLUDE_DIR - The Inja include directory (single)
#   Inja_INCLUDE_DIRS - The Inja include directories (list)
#   Inja::inja - Imported target for Inja
#
# The module first tries pkg-config, then falls back to direct path search.

# Try pkg-config first
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_Inja QUIET inja)
endif()

# Use pkg-config results if available, otherwise search directly
if(PC_Inja_FOUND)
    # pkg-config found it - extract include dirs
    set(Inja_INCLUDE_DIRS ${PC_Inja_INCLUDE_DIRS})
    # FPHSA expects a single directory, so extract the first one (if any)
    if(Inja_INCLUDE_DIRS)
        list(GET Inja_INCLUDE_DIRS 0 Inja_INCLUDE_DIR)
    endif()
else()
    find_path(Inja_INCLUDE_DIR
        NAMES inja/inja.hpp
        HINTS
            # Homebrew (Apple Silicon)
            /opt/homebrew/opt/inja
            # Homebrew (Intel)
            /usr/local/opt/inja
            # Environment variable override
            ENV INJA_ROOT
        PATH_SUFFIXES include
        DOC "Inja template engine include directory"
    )
    # Set INCLUDE_DIRS from the single directory for consistency
    if(Inja_INCLUDE_DIR)
        set(Inja_INCLUDE_DIRS ${Inja_INCLUDE_DIR})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Inja
    REQUIRED_VARS Inja_INCLUDE_DIR
    FAIL_MESSAGE "Inja template engine not found. Install via:
    APT:      apt install inja-dev
    Homebrew: brew install inja
    Conan:    conan install inja/3.5.0
    vcpkg:    vcpkg install inja"
)

if(Inja_FOUND AND NOT TARGET Inja::inja)
    add_library(Inja::inja INTERFACE IMPORTED)
    set_property(TARGET Inja::inja PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${Inja_INCLUDE_DIRS}
    )
endif()

mark_as_advanced(Inja_INCLUDE_DIR Inja_INCLUDE_DIRS)

