# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Script to generate QDoc coverage HTML reports using genhtml

cmake_minimum_required(VERSION 3.16)

if(NOT QDOC_BUILD_DIR OR NOT QDOC_SOURCE_DIR OR NOT GENHTML_EXECUTABLE)
    message(FATAL_ERROR "Required variables not set: QDOC_BUILD_DIR, QDOC_SOURCE_DIR, GENHTML_EXECUTABLE")
endif()

message(STATUS "Generating QDoc coverage HTML report...")

set(COVERAGE_INFO_FILE "${QDOC_BUILD_DIR}/coverage_data/qdoc_filtered.info")
set(COVERAGE_HTML_DIR "${QDOC_BUILD_DIR}/coverage_report")

# Check for filtered coverage file, fall back to all.info if needed
if(NOT EXISTS "${COVERAGE_INFO_FILE}")
    set(COVERAGE_INFO_FILE "${QDOC_BUILD_DIR}/coverage_data/all.info")
    if(NOT EXISTS "${COVERAGE_INFO_FILE}")
        message(FATAL_ERROR "No coverage data files found. Expected: qdoc_filtered.info or all.info")
    endif()
    message(STATUS "Using fallback coverage file: ${COVERAGE_INFO_FILE}")
endif()

file(MAKE_DIRECTORY "${COVERAGE_HTML_DIR}")

file(COPY "${CMAKE_CURRENT_LIST_DIR}/coverage-prolog.html"
     DESTINATION "${COVERAGE_HTML_DIR}/")

# Generate HTML report
execute_process(
    COMMAND ${GENHTML_EXECUTABLE}
        "${COVERAGE_INFO_FILE}"
        --output-directory "${COVERAGE_HTML_DIR}"
        --title "QDoc Test Coverage Report"
        --show-details
        --html-prolog "${COVERAGE_HTML_DIR}/coverage-prolog.html"
        --legend
        --branch-coverage
        --ignore-errors inconsistent,corrupt
    RESULT_VARIABLE GENHTML_RESULT
    OUTPUT_VARIABLE GENHTML_OUTPUT
    ERROR_VARIABLE GENHTML_ERROR
)

if(GENHTML_RESULT EQUAL 0)
    message(STATUS "Coverage HTML report generated successfully")
    message(STATUS "Report location: ${COVERAGE_HTML_DIR}")
    message(STATUS "Open in browser: file://${COVERAGE_HTML_DIR}/index.html")

    string(TIMESTAMP GEN_TIME "%Y-%m-%d %H:%M:%S %Z")
    file(WRITE "${COVERAGE_HTML_DIR}/COVERAGE_SUMMARY.txt"
        "QDoc Test Coverage Summary\n"
        "===========================\n\n"
        "Generated: ${GEN_TIME}\n"
        "Report: file://${COVERAGE_HTML_DIR}/index.html\n"
        "Source: ${QDOC_SOURCE_DIR}\n"
        "Build:  ${QDOC_BUILD_DIR}\n\n"
        "Coverage data collected from:\n"
        "- QDoc binary execution during tests\n"
        "- Test executable runs\n"
        "- End-to-end documentation generation\n\n"
        "Report includes:\n"
        "- Line coverage\n"
        "- Function coverage\n"
        "- Branch coverage\n"
    )

    message(STATUS "Coverage summary: ${COVERAGE_HTML_DIR}/COVERAGE_SUMMARY.txt")
else()
    message(FATAL_ERROR "Failed to generate HTML coverage report: ${GENHTML_ERROR}")
endif()

