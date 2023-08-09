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

# Remove all nonexistent files from ARGN and store the result in out_var.
#    filter_nonexistent_files(existing_files foo.txt bar.cpp)
#    -> foo.txt (if foo.txt exists and bar.cpp does not)
function(filter_nonexistent_files out_var)
    # Filter out non-existent (generated) source files
    set(existing_sources "")
    foreach(path IN LISTS ARGN)
        if(EXISTS "${path}")
            list(APPEND existing_sources "${path}")
        endif()
    endforeach()
    set("${out_var}" "${existing_sources}" PARENT_SCOPE)
endfunction()

get_filename_component(project_root "${lupdate_project_file}" DIRECTORY)

# Make relative paths absolute to the project root
set(path_variables sources include_paths translations)
foreach(i RANGE 1 ${lupdate_subproject_count})
    list(APPEND path_variables
        subproject${i}_include_paths
        subproject${i}_sources
    )
endforeach()
foreach(path_var IN LISTS path_variables)
    set(absolute_${path_var} "")
    foreach(path IN LISTS lupdate_${path_var})
        if(NOT IS_ABSOLUTE "${path}")
            if(path_var MATCHES "^subproject[0-9]+_")
                set(base_dir "${lupdate_${CMAKE_MATCH_0}source_dir}")
            else()
                set(base_dir "${project_root}")
            endif()
            get_filename_component(path "${path}" ABSOLUTE BASE_DIR "${base_dir}")
        endif()
        list(APPEND absolute_${path_var} "${path}")
    endforeach()
endforeach()

filter_nonexistent_files(existing_sources ${absolute_sources})
list_to_json_array("${existing_sources}" json_sources)
list_to_json_array("${absolute_include_paths}" json_include_paths)
list_to_json_array("${absolute_translations}" json_translations)

set(content "{
  \"projectFile\": \"${lupdate_project_file}\",
  \"includePaths\": ${json_include_paths},
  \"sources\": ${json_sources},
  \"translations\": ${json_translations},
  \"subProjects\": [
")
foreach(i RANGE 1 ${lupdate_subproject_count})
    filter_nonexistent_files(existing_sources ${absolute_subproject${i}_sources})
    list_to_json_array("${existing_sources}" json_sources)
    list_to_json_array("${absolute_subproject${i}_include_paths}" json_include_paths)
    string(APPEND content "    {
      \"projectFile\": \"${lupdate_subproject${i}_source_dir}/CMakeLists.txt\",
      \"includePaths\": ${json_include_paths},
      \"sources\": ${json_sources}
    }")
    if(i LESS lupdate_subproject_count)
        string(APPEND content ",")
    endif()
    string(APPEND content "\n")
endforeach()
string(APPEND content "  ]
}
")
file(WRITE "${OUT_FILE}" "${content}")
