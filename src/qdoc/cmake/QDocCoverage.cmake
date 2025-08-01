# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# QDoc test coverage support using gcov/lcov
# Note: Feature definition is in qttools/configure.cmake

if(NOT QT_FEATURE_qdoc_coverage)
  return()
endif()

# If we get here, the feature was enabled, so dependencies should be available
# But let's double-check and provide better error messaging

find_program(LCOV_EXECUTABLE lcov DOC "Path to lcov executable")
find_program(GENHTML_EXECUTABLE genhtml DOC "Path to genhtml executable")

if(NOT LCOV_EXECUTABLE)
    message(FATAL_ERROR "QT_FEATURE_qdoc_coverage is ON, but 'lcov' was not found. "
                       "This should not happen as the feature checks for dependencies. "
                       "Please reconfigure with a clean build directory.")
endif()

# Check lcov version for branch coverage support
execute_process(
    COMMAND ${LCOV_EXECUTABLE} --version
    OUTPUT_VARIABLE LCOV_VERSION_OUTPUT
    ERROR_QUIET
)
if(LCOV_VERSION_OUTPUT MATCHES "LCOV version ([0-9]+)\\.([0-9]+)")
    set(LCOV_MAJOR ${CMAKE_MATCH_1})
    set(LCOV_MINOR ${CMAKE_MATCH_2})
    message(STATUS "Found lcov version ${LCOV_MAJOR}.${LCOV_MINOR}")
    if(LCOV_MAJOR LESS 2)
        message(WARNING "lcov version ${LCOV_MAJOR}.${LCOV_MINOR} found. Version >= 2.0 recommended for full branch coverage support.")
    endif()
else()
    message(WARNING "Could not determine lcov version. Some features may not work correctly.")
endif()

if(NOT GENHTML_EXECUTABLE)
    message(WARNING "genhtml not found. HTML reports will not be available, "
                   "but coverage data collection will still work.")
else()
    message(STATUS "Found genhtml - HTML reports will be available")
endif()

add_custom_target(qdoc_coverage_collect
    COMMAND ${CMAKE_COMMAND}
        -DQDOC_BUILD_DIR=${CMAKE_BINARY_DIR}
        -DQDOC_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DLCOV_EXECUTABLE=${LCOV_EXECUTABLE}
        -P ${CMAKE_CURRENT_LIST_DIR}/QDocCoverageCollect.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Collecting QDoc coverage data"
    USES_TERMINAL
    VERBATIM
)

if(GENHTML_EXECUTABLE)
    add_custom_target(qdoc_coverage_report
        COMMAND ${CMAKE_COMMAND}
            -DQDOC_BUILD_DIR=${CMAKE_BINARY_DIR}
            -DQDOC_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DGENHTML_EXECUTABLE=${GENHTML_EXECUTABLE}
            -P ${CMAKE_CURRENT_LIST_DIR}/QDocCoverageReport.cmake
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating QDoc coverage HTML report"
        USES_TERMINAL
        VERBATIM
    )
endif()

add_custom_target(qdoc_coverage_setup
    COMMAND ${CMAKE_COMMAND} -E remove_directory coverage_data
    COMMAND ${CMAKE_COMMAND} -E make_directory coverage_data
    COMMAND ${LCOV_EXECUTABLE} --zerocounters --directory "${CMAKE_BINARY_DIR}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Setting up coverage data directory and zeroing counters"
    USES_TERMINAL
    VERBATIM
)

add_dependencies(qdoc_coverage_collect qdoc_coverage_setup)

if(QT_BUILD_TESTS)
    add_custom_target(qdoc_run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure -L "src/qdoc/.*"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running QDoc tests"
        USES_TERMINAL
        VERBATIM
    )

    if(TARGET qdoc)
        add_dependencies(qdoc_run_tests qdoc)
    endif()

    # Add dependencies on specific QDoc test targets
    set(QDOC_TEST_TARGETS
        tst_QDoc_Catch_Generators
        tst_QDoc
        tst_config
        tst_inodeinterface
        tst_qdoccommandlineparser
        tst_nativeenum
        tst_utilities
        tst_generatedOutput
        tst_validateQdocOutputFiles
    )

    foreach(test_target ${QDOC_TEST_TARGETS})
        if(TARGET ${test_target})
            add_dependencies(qdoc_run_tests ${test_target})
        endif()
    endforeach()

    add_dependencies(qdoc_coverage_collect qdoc_run_tests)
endif()

# Main user-facing target
add_custom_target(qdoc_test_with_coverage
    COMMENT "Run QDoc tests, collect coverage, and generate HTML report"
)

# Set up clean dependency chain: main -> report -> collect
if(TARGET qdoc_coverage_report)
    add_dependencies(qdoc_test_with_coverage qdoc_coverage_report)
    add_dependencies(qdoc_coverage_report qdoc_coverage_collect)
else()
    # If no HTML report target, main depends directly on collect
    add_dependencies(qdoc_test_with_coverage qdoc_coverage_collect)
endif()

message(STATUS "QDoc coverage targets available:")
message(STATUS "  qdoc_test_with_coverage - Run tests, collect coverage, and generate HTML report")
message(STATUS "  qdoc_coverage_collect   - Collect coverage data from previous test runs")
if(TARGET qdoc_coverage_report)
    message(STATUS "  qdoc_coverage_report    - Generate HTML report from collected coverage data")
endif()
message(STATUS "  qdoc_coverage_check_baseline - Check coverage against baseline and thresholds")
message(STATUS "  qdoc_coverage_set_baseline   - Set coverage baseline from current results")


