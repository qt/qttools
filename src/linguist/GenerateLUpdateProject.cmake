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

# Remove source files that are unsuitable input for lupdate.
#    filter_unsuitable_lupdate_input(sources main.cpp foo_de.qm bar.qml whatever_metatypes.json)
#    -> main.cpp bar.qml
function(filter_unsuitable_lupdate_input out_var)
    set(result ${ARGN})
    list(FILTER result EXCLUDE REGEX "\\.(qm|json)$")
    set("${out_var}" "${result}" PARENT_SCOPE)
endfunction()

# Remove ui_foo.h for each foo.ui file found in the sources.
#    filter_generated_ui_headers(existing_files .../src/foo.ui .../target_autogen/include/ui_foo.h)
#    -> .../src/foo.ui
function(filter_generated_ui_headers out_var)
    set(ui_file_paths ${ARGN})
    list(FILTER ui_file_paths INCLUDE REGEX "/[^/]+\\.ui$")

    set(filter_regex "")
    foreach(file_path IN LISTS ui_file_paths)
        get_filename_component(file_name "${file_path}" NAME_WLE)
        if(NOT "${filter_regex}" STREQUAL "")
            string(APPEND filter_regex "|")
        endif()
        string(APPEND filter_regex "(/ui_${file_name}\\.h$)")
    endforeach()

    set(result ${ARGN})
    if(NOT "${filter_regex}" STREQUAL "")
        list(FILTER result EXCLUDE REGEX ${filter_regex})
    endif()

    set("${out_var}" "${result}" PARENT_SCOPE)
endfunction()

function(prepare_json_sources out_var)
    filter_nonexistent_files(sources ${ARGN})
    filter_unsuitable_lupdate_input(sources ${sources})
    filter_generated_ui_headers(sources ${sources})
    list_to_json_array("${sources}" result)
    set("${out_var}" "${result}" PARENT_SCOPE)
endfunction()

get_filename_component(project_root "${lupdate_project_file}" DIRECTORY)

# Make relative paths absolute to the project root
set(path_variables sources include_paths translations)
if(lupdate_subproject_count GREATER 0)
    foreach(i RANGE 1 ${lupdate_subproject_count})
        list(APPEND path_variables
            subproject${i}_include_paths
            subproject${i}_sources
            subproject${i}_excluded
        )
    endforeach()
endif()
foreach(path_var IN LISTS path_variables)
    set(absolute_${path_var} "")
    foreach(path IN LISTS lupdate_${path_var})
        if(path STREQUAL "")
            continue()
        endif()
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

prepare_json_sources(json_sources ${absolute_sources})
list_to_json_array("${absolute_include_paths}" json_include_paths)
list_to_json_array("${absolute_translations}" json_translations)

set(content "{
  \"projectFile\": \"${lupdate_project_file}\",
  \"includePaths\": ${json_include_paths},
  \"sources\": ${json_sources},
  \"translations\": ${json_translations},
  \"subProjects\": [
")
if(lupdate_subproject_count GREATER 0)
    foreach(i RANGE 1 ${lupdate_subproject_count})
        prepare_json_sources(json_sources ${absolute_subproject${i}_sources})
        list_to_json_array("${absolute_subproject${i}_include_paths}" json_include_paths)
        list_to_json_array("${absolute_subproject${i}_excluded}" json_sources_exclusions)
        string(APPEND content "    {
      \"projectFile\": \"${lupdate_subproject${i}_source_dir}/CMakeLists.txt\",
      \"includePaths\": ${json_include_paths},
      \"sources\": ${json_sources},
      \"excluded\": ${json_sources_exclusions}
    }")
        if(i LESS lupdate_subproject_count)
            string(APPEND content ",")
        endif()
        string(APPEND content "\n")
    endforeach()
endif()
string(APPEND content "  ]
}
")
file(WRITE "${OUT_FILE}" "${content}")
