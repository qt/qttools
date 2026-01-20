# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

message("Updating the .ts files...")
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build . --target update_translations
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE process_result
)
if(NOT process_result EQUAL "0")
    message(FATAL_ERROR "Command error: ${process_result}")
endif()

function(check_ts_file ts_file)
    message("Checking the content of '${ts_file}'...")
    file(READ "${ts_file}" ts_file_content)

    set(expected_strings
        "<source>%n argument(s) passed</source>"
    )
    if(ts_file MATCHES "^lib2_en\\.ts$")
        list(APPEND expected_strings
            "<numerusform>one argument passed</numerusform>"
        )
    endif()
    foreach(needle IN LISTS expected_strings)
        string(FIND "${ts_file_content}" "${needle}" pos)
        if(pos EQUAL "-1")
            message(FATAL_ERROR
                "Expected string '${needle}' was not found in '${ts_file}'. "
                "The file content is:\n${ts_file_content}"
            )
        endif()
    endforeach()

    set(forbidden_strings
        "<source>We must not see this in the source language"
    )
    foreach(needle IN LISTS forbidden_strings)
        string(FIND "${ts_file_content}" "${needle}" pos)
        if(NOT pos EQUAL "-1")
            message(FATAL_ERROR
                "Excluded string '${needle}' was found in '${ts_file}'. "
                "The file content is:\n${ts_file_content}"
            )
        endif()
    endforeach()

    # Check that location tags exist and reference valid files
    message("Checking source locations in '${ts_file}'...")
    string(REGEX MATCHALL "<location filename=\"([^\"]+)\" line=\"([0-9]+)\"/>" location_matches "${ts_file_content}")
    if(NOT location_matches)
        message(FATAL_ERROR
            "No location tags found in '${ts_file}'. "
            "The file content is:\n${ts_file_content}"
        )
    endif()

    # Extract and verify each location's file exists
    string(REGEX MATCHALL "filename=\"([^\"]+)\"" filename_matches "${ts_file_content}")
    foreach(filename_match IN LISTS filename_matches)
        string(REGEX REPLACE "filename=\"([^\"]+)\"" "\\1" source_file "${filename_match}")
        if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${source_file}")
            message(FATAL_ERROR
                "Source location '${source_file}' referenced in '${ts_file}' does not exist. "
                "Expected path: ${CMAKE_CURRENT_BINARY_DIR}/${source_file}"
            )
        endif()
        message("  Found valid location: ${source_file}")
    endforeach()
endfunction()

file(READ ts_files.txt ts_files_to_check)
if(ts_files_to_check STREQUAL "")
    message(FATAL_ERROR "No .ts files to check.")
endif()
foreach(ts_file IN LISTS ts_files_to_check)
    check_ts_file("${ts_file}")
endforeach()
