# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generate an lupdate project file in JSON format.
#
# This file is to be used in CMake script mode with the following variables set:
#
# IN_FILE: .cmake file that sets lupdate_* variables
# OUT_FILE: lupdate project .json file

include("${IN_FILE}")

# Converts a CMake list into a string containing a JSON array
#    a,b,c -> [ "a", "b", "c" ]
function(list_to_json_array srcList jsonList)
    set(quotedList "")
    foreach(entry ${srcList})
        list(APPEND quotedList "\"${entry}\"")
    endforeach()
    list(JOIN quotedList ", " joinedList)
    set(${jsonList} "[ ${joinedList} ]" PARENT_SCOPE)
endfunction()

get_filename_component(project_root "${lupdate_project_file}" DIRECTORY)

# Make relative paths absolute to the project root
foreach(path_var sources include_paths translations)
    set(absolute_${path_var} "")
    foreach(path IN LISTS lupdate_${path_var})
        if(NOT IS_ABSOLUTE "${path}")
            get_filename_component(path "${path}" ABSOLUTE BASE_DIR "${project_root}")
        endif()
        list(APPEND absolute_${path_var} "${path}")
    endforeach()
endforeach()

# Filter out non-existent (generated) source files
set(existing_sources "")
foreach(path IN LISTS absolute_sources)
    if(EXISTS "${path}")
        list(APPEND existing_sources "${path}")
    endif()
endforeach()

list_to_json_array("${existing_sources}" json_sources)
list_to_json_array("${absolute_include_paths}" json_include_paths)
list_to_json_array("${absolute_translations}" json_translations)

set(content "{"
    "  \"projectFile\": \"${lupdate_project_file}\","
    "  \"includePaths\": ${json_include_paths},"
    "  \"sources\": ${json_sources},"
    "  \"translations\": ${json_translations}"
    "}\n")

list(JOIN content "\n" content)
file(WRITE "${OUT_FILE}" "${content}")
