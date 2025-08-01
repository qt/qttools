# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Script to collect QDoc coverage data from multiple test runs
# This handles the challenge of QDoc's end-to-end tests spawning separate processes

cmake_minimum_required(VERSION 3.16)

if(NOT QDOC_BUILD_DIR OR NOT QDOC_SOURCE_DIR OR NOT LCOV_EXECUTABLE)
    message(FATAL_ERROR "Required variables not set: QDOC_BUILD_DIR, QDOC_SOURCE_DIR, LCOV_EXECUTABLE")
endif()

message(STATUS "Collecting QDoc coverage data...")
message(STATUS "Build directory: ${QDOC_BUILD_DIR}")
message(STATUS "Source directory: ${QDOC_SOURCE_DIR}")

file(MAKE_DIRECTORY "${QDOC_BUILD_DIR}/coverage_data")

# Capture all coverage data from the entire build tree (generator-agnostic)
# Use --ignore-errors to handle inconsistencies from multi-process tests like
# tst_generatedOutput and tst_validateQdocOutputFiles that spawn multiple QDoc processes
# Enable branch coverage to track both true/false paths of conditionals
execute_process(
    COMMAND ${LCOV_EXECUTABLE}
        --capture
        --directory "${QDOC_BUILD_DIR}"
        --base-directory "${QDOC_SOURCE_DIR}"
        --output-file "${QDOC_BUILD_DIR}/coverage_data/all.info"
        --ignore-errors inconsistent,mismatch
        --branch-coverage
    RESULT_VARIABLE LCOV_RESULT
    OUTPUT_VARIABLE LCOV_OUTPUT
    ERROR_VARIABLE LCOV_ERROR
)

if(LCOV_RESULT EQUAL 0)
    message(STATUS "Captured all coverage data from build tree")

    execute_process(
        COMMAND ${LCOV_EXECUTABLE}
            --extract "${QDOC_BUILD_DIR}/coverage_data/all.info"
            "${QDOC_SOURCE_DIR}/src/qdoc/*"
            --output-file "${QDOC_BUILD_DIR}/coverage_data/qdoc_filtered.info"
            --ignore-errors inconsistent,mismatch
            --branch-coverage
            RESULT_VARIABLE LCOV_RESULT
        OUTPUT_VARIABLE LCOV_OUTPUT
        ERROR_VARIABLE LCOV_ERROR
    )

    if(LCOV_RESULT EQUAL 0)
        message(STATUS "Extracted coverage data for QDoc sources")
        set(FINAL_COVERAGE_FILE "${QDOC_BUILD_DIR}/coverage_data/qdoc_filtered.info")
    else()
        message(WARNING "Failed to extract coverage data, using all: ${LCOV_ERROR}")
        set(FINAL_COVERAGE_FILE "${QDOC_BUILD_DIR}/coverage_data/all.info")
    endif()
else()
    message(WARNING "Failed to capture coverage data: ${LCOV_ERROR}")
endif()

# Final status reporting
if(EXISTS "${QDOC_BUILD_DIR}/coverage_data/qdoc_filtered.info")
    message(STATUS "Final coverage file: ${QDOC_BUILD_DIR}/coverage_data/qdoc_filtered.info")
elseif(EXISTS "${QDOC_BUILD_DIR}/coverage_data/all.info")
    message(STATUS "Final coverage file: ${QDOC_BUILD_DIR}/coverage_data/all.info")
else()
    message(WARNING "No coverage data files found. Make sure to run tests with coverage enabled.")
endif()

message(STATUS "Coverage data collection complete")
