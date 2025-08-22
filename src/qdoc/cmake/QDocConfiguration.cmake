# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# QDoc-specific configuration variables

# Minimum supported Clang version for QDoc
set(QDOC_MINIMUM_CLANG_VERSION "17")

# List of explicitly supported Clang versions for QDoc
set(QDOC_SUPPORTED_CLANG_VERSIONS
    "21.1" "20.1" "19.1" "18.1" "17.0"
)

# Check for QDoc coverage dependencies
find_program(LCOV_EXECUTABLE lcov DOC "Path to lcov executable")
set(QDOC_COVERAGE_DEPS_FOUND FALSE)
if(LCOV_EXECUTABLE)
    execute_process(
        COMMAND ${LCOV_EXECUTABLE} --version
        OUTPUT_VARIABLE LCOV_VERSION_OUTPUT
        ERROR_QUIET
    )
    if(LCOV_VERSION_OUTPUT MATCHES "LCOV version ([0-9]+)\\.([0-9]+)")
        set(LCOV_MAJOR ${CMAKE_MATCH_1})
        if(LCOV_MAJOR GREATER_EQUAL 1)
            set(QDOC_COVERAGE_DEPS_FOUND TRUE)
        endif()
    endif()
endif()

