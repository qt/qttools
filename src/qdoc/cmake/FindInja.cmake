# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# FindInja.cmake - Find the Inja template engine library
#
# This module defines:
#   Inja_FOUND - System has Inja
#   Inja_INCLUDE_DIRS - The Inja include directories
#   Inja::inja - Imported target for Inja

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

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Inja
    REQUIRED_VARS Inja_INCLUDE_DIR
    FAIL_MESSAGE "Inja template engine not found. Install via:
    Homebrew: brew install inja
    Conan:    conan install inja/3.5.0
    vcpkg:    vcpkg install inja"
)

if(Inja_FOUND)
    set(Inja_INCLUDE_DIRS ${Inja_INCLUDE_DIR})

    if(NOT TARGET Inja::inja)
        add_library(Inja::inja INTERFACE IMPORTED)
        set_target_properties(Inja::inja PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${Inja_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(Inja_INCLUDE_DIR)

