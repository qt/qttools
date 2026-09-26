# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# QDoc-specific configuration variables

# Minimum supported Clang version for QDoc
set(QDOC_MINIMUM_CLANG_VERSION "17")

# List of explicitly supported Clang versions for QDoc
set(QDOC_SUPPORTED_CLANG_VERSIONS
    "23.1" "22.1" "21.1" "20.1" "19.1" "18.1" "17.0.6"
)

# The QDoc manual documents the supported Clang versions for each Qt
# release in a table in qdoc-clang-versions.qdocinc. The row for the
# current Qt release is validated against QDOC_SUPPORTED_CLANG_VERSIONS,
# so that a change to the supported versions without a matching
# documentation update produces a warning at configure time. Rows for
# earlier releases are a historical record and are not validated.
set(__qdoc_clang_versions_doc
    "${CMAKE_CURRENT_LIST_DIR}/../qdoc/doc/qdoc-guide/qdoc-clang-versions.qdocinc")
if(QT_REPO_MODULE_VERSION AND EXISTS "${__qdoc_clang_versions_doc}")
    string(REGEX MATCH "^[0-9]+\\.[0-9]+" __qdoc_qt_version "${QT_REPO_MODULE_VERSION}")
    set(__qdoc_clang_doc_floor "")
    set(__qdoc_clang_doc_ceiling "")
    foreach(__qdoc_clang_version IN LISTS QDOC_SUPPORTED_CLANG_VERSIONS)
        if(__qdoc_clang_doc_floor STREQUAL "" OR
                __qdoc_clang_version VERSION_LESS __qdoc_clang_doc_floor)
            set(__qdoc_clang_doc_floor "${__qdoc_clang_version}")
        endif()
        if(__qdoc_clang_doc_ceiling STREQUAL "" OR
                __qdoc_clang_version VERSION_GREATER __qdoc_clang_doc_ceiling)
            set(__qdoc_clang_doc_ceiling "${__qdoc_clang_version}")
        endif()
    endforeach()
    set(__qdoc_expected_row
        "\\row \\li ${__qdoc_qt_version} \\li ${__qdoc_clang_doc_floor} - ${__qdoc_clang_doc_ceiling}")
    file(READ "${__qdoc_clang_versions_doc}" __qdoc_clang_doc_content)
    string(FIND "${__qdoc_clang_doc_content}" "${__qdoc_expected_row}"
        __qdoc_clang_doc_row_position)
    if(__qdoc_clang_doc_row_position EQUAL -1)
        message(WARNING
            "The Clang version table in the QDoc manual does not match QDoc's "
            "declared Clang support for Qt ${__qdoc_qt_version}. The row for "
            "Qt ${__qdoc_qt_version} in\n  ${__qdoc_clang_versions_doc}\n"
            "must read:\n  ${__qdoc_expected_row}\n"
            "This happens when QT_REPO_MODULE_VERSION is bumped, or when "
            "QDOC_MINIMUM_CLANG_VERSION or QDOC_SUPPORTED_CLANG_VERSIONS "
            "change. Add or update the row by running:\n"
            "  cmake -P src/qdoc/cmake/GenerateQdocClangVersions.cmake")
    endif()
    unset(__qdoc_clang_doc_content)
    unset(__qdoc_expected_row)
    unset(__qdoc_clang_doc_row_position)
    unset(__qdoc_qt_version)
    unset(__qdoc_clang_doc_floor)
    unset(__qdoc_clang_doc_ceiling)
    unset(__qdoc_clang_version)
    unset(__qdoc_clang_versions_doc)
endif()

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

# Check if user explicitly disabled QDoc via -no-feature-qdoc
# When TRUE, QDoc dependency warnings should be suppressed
if(NOT QT_CONFIGURE_RUNNING AND NOT DEFINED QDOC_EXPLICITLY_DISABLED)
    if(DEFINED FEATURE_qdoc AND NOT FEATURE_qdoc)
        set(QDOC_EXPLICITLY_DISABLED TRUE CACHE INTERNAL
            "QDoc was explicitly disabled by user via -no-feature-qdoc")
    else()
        set(QDOC_EXPLICITLY_DISABLED FALSE CACHE INTERNAL
            "QDoc was not explicitly disabled by user")
    endif()
endif()
