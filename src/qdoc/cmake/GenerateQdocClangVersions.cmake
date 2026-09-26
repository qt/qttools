# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Regenerate the row for the current Qt release in the QDoc manual's
# supported-Clang-versions table (qdoc-clang-versions.qdocinc) from the
# QDOC_SUPPORTED_CLANG_VERSIONS variable in QDocConfiguration.cmake.
#
# Usage:
#   cmake -P GenerateQdocClangVersions.cmake
#
# Optional -D arguments (the defaults are relative to this file):
#   QDOC_CONFIGURATION_FILE  Path to QDocConfiguration.cmake
#   QDOC_VERSIONS_DOC        Path to qdoc-clang-versions.qdocinc
#   QDOC_QT_VERSION          Qt release for the row to update, as
#                            "major.minor". If not given, the current
#                            release is read from the QT_REPO_MODULE_VERSION
#                            variable in the repository's .cmake.conf file.
#
# The row for the given release is replaced in place. If the table does
# not yet contain a row for the release, one is inserted before
# \endtable. All other content is left untouched.

if(NOT DEFINED QDOC_CONFIGURATION_FILE)
    set(QDOC_CONFIGURATION_FILE "${CMAKE_CURRENT_LIST_DIR}/QDocConfiguration.cmake")
endif()
if(NOT DEFINED QDOC_VERSIONS_DOC)
    set(QDOC_VERSIONS_DOC
        "${CMAKE_CURRENT_LIST_DIR}/../qdoc/doc/qdoc-guide/qdoc-clang-versions.qdocinc")
endif()
get_filename_component(QDOC_CONFIGURATION_FILE "${QDOC_CONFIGURATION_FILE}" ABSOLUTE)
get_filename_component(QDOC_VERSIONS_DOC "${QDOC_VERSIONS_DOC}" ABSOLUTE)

if(NOT EXISTS "${QDOC_CONFIGURATION_FILE}")
    message(FATAL_ERROR "QDocConfiguration.cmake not found: ${QDOC_CONFIGURATION_FILE}")
endif()
if(NOT EXISTS "${QDOC_VERSIONS_DOC}")
    message(FATAL_ERROR "Clang versions documentation file not found: ${QDOC_VERSIONS_DOC}\n"
        "Commit the qdoc-clang-versions.qdocinc file first.")
endif()

if(NOT DEFINED QDOC_QT_VERSION)
    get_filename_component(__qdoc_conf_dir "${QDOC_CONFIGURATION_FILE}" DIRECTORY)
    get_filename_component(__qdoc_repo_root "${__qdoc_conf_dir}/../../.." ABSOLUTE)
    unset(__qdoc_conf_dir)
    if(NOT EXISTS "${__qdoc_repo_root}/.cmake.conf")
        message(FATAL_ERROR "No .cmake.conf found at ${__qdoc_repo_root}; "
            "pass -DQDOC_QT_VERSION=major.minor instead.")
    endif()
    file(READ "${__qdoc_repo_root}/.cmake.conf" __qdoc_cmake_conf)
    string(REGEX MATCH "set\\(QT_REPO_MODULE_VERSION[ ]+\"[^\"]+\""
        __qdoc_version_line "${__qdoc_cmake_conf}")
    string(REGEX MATCH "[0-9]+\\.[0-9]+" __qdoc_version "${__qdoc_version_line}")
    string(REGEX MATCH "^[0-9]+\\.[0-9]+" QDOC_QT_VERSION "${__qdoc_version}")
    unset(__qdoc_cmake_conf)
    unset(__qdoc_version_line)
    unset(__qdoc_version)
    unset(__qdoc_repo_root)
endif()

# Read QDOC_SUPPORTED_CLANG_VERSIONS. The list is parsed line by line so
# that this script can run in CMake script mode, where QDocConfiguration.cmake
# must not be included.
file(READ "${QDOC_CONFIGURATION_FILE}" __qdoc_config_content)
string(REPLACE "\n" ";" __qdoc_config_lines "${__qdoc_config_content}")
set(__qdoc_versions "")
set(__qdoc_in_list FALSE)
foreach(__qdoc_line IN LISTS __qdoc_config_lines)
    if(__qdoc_line MATCHES "set\\(QDOC_SUPPORTED_CLANG_VERSIONS")
        set(__qdoc_in_list TRUE)
    elseif(__qdoc_in_list)
        if(__qdoc_line MATCHES "^[ ]*\\)")
            set(__qdoc_in_list FALSE)
        else()
            string(REGEX MATCHALL "\"[^\"]+\"" __qdoc_quoted "${__qdoc_line}")
            foreach(__qdoc_v IN LISTS __qdoc_quoted)
                string(REPLACE "\"" "" __qdoc_v "${__qdoc_v}")
                list(APPEND __qdoc_versions "${__qdoc_v}")
            endforeach()
            unset(__qdoc_quoted)
            unset(__qdoc_v)
        endif()
    endif()
endforeach()
unset(__qdoc_config_content)
unset(__qdoc_config_lines)
unset(__qdoc_line)
unset(__qdoc_in_list)

if(__qdoc_versions STREQUAL "")
    message(FATAL_ERROR "Could not parse QDOC_SUPPORTED_CLANG_VERSIONS from "
        "${QDOC_CONFIGURATION_FILE}")
endif()

set(__qdoc_floor "")
set(__qdoc_ceiling "")
foreach(__qdoc_version IN LISTS __qdoc_versions)
    if(__qdoc_floor STREQUAL "" OR
            __qdoc_version VERSION_LESS __qdoc_floor)
        set(__qdoc_floor "${__qdoc_version}")
    endif()
    if(__qdoc_ceiling STREQUAL "" OR
            __qdoc_version VERSION_GREATER __qdoc_ceiling)
        set(__qdoc_ceiling "${__qdoc_version}")
    endif()
endforeach()
unset(__qdoc_versions)
unset(__qdoc_version)

set(__qdoc_row "\\row \\li ${QDOC_QT_VERSION} \\li ${__qdoc_floor} - ${__qdoc_ceiling}")
set(__qdoc_row_prefix "\\row \\li ${QDOC_QT_VERSION} \\li")
unset(__qdoc_floor)
unset(__qdoc_ceiling)

file(READ "${QDOC_VERSIONS_DOC}" __qdoc_doc_content)
string(REPLACE "\n" ";" __qdoc_doc_lines "${__qdoc_doc_content}")
set(__qdoc_new_lines "")
set(__qdoc_row_replaced FALSE)
foreach(__qdoc_line IN LISTS __qdoc_doc_lines)
    if(__qdoc_row_replaced)
        list(APPEND __qdoc_new_lines "${__qdoc_line}")
        continue()
    endif()
    if(__qdoc_line MATCHES "endtable")
        if(NOT __qdoc_row_replaced)
            list(APPEND __qdoc_new_lines "    ${__qdoc_row}")
        endif()
        list(APPEND __qdoc_new_lines "${__qdoc_line}")
    else()
        string(FIND "${__qdoc_line}" "${__qdoc_row_prefix}" __qdoc_row_position)
        if(__qdoc_row_position GREATER -1)
            list(APPEND __qdoc_new_lines "    ${__qdoc_row}")
            set(__qdoc_row_replaced TRUE)
        else()
            list(APPEND __qdoc_new_lines "${__qdoc_line}")
        endif()
        unset(__qdoc_row_position)
    endif()
endforeach()
unset(__qdoc_doc_content)
unset(__qdoc_doc_lines)
unset(__qdoc_line)

# A file that ends with a newline yields a trailing empty element when
# split, so the joined content retains the original line structure,
# including the final newline.
string(REPLACE ";" "\n" __qdoc_new_content "${__qdoc_new_lines}")

file(READ "${QDOC_VERSIONS_DOC}" __qdoc_old_content)
if(__qdoc_new_content STREQUAL __qdoc_old_content)
    message(STATUS "Clang versions documentation already up to date "
        "(Qt ${QDOC_QT_VERSION}).")
else()
    file(WRITE "${QDOC_VERSIONS_DOC}" "${__qdoc_new_content}")
    message(STATUS "Updated Clang versions documentation for Qt ${QDOC_QT_VERSION}:\n"
        "  ${__qdoc_row}\n"
        "Do not forget to stage ${QDOC_VERSIONS_DOC} in your commit.")
    unset(__qdoc_row)
endif()
unset(__qdoc_new_lines)
unset(__qdoc_new_content)
unset(__qdoc_old_content)
unset(__qdoc_row_replaced)
