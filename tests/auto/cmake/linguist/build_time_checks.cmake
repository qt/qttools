# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include_guard()

# Check whether ${file_path} is generated during the build phase.
# We do this by creating a custom command that creates a copy of the file.
# If the file_path is not known to the build system, ninja will complain.
# If the file_path is known to the build system but not created, the copy command fails.
function(check_file_is_built file_path)
    set(out_file "${file_path}.copy")
    add_custom_command(
        OUTPUT "${out_file}"
        DEPENDS "${file_path}"
        COMMAND "${CMAKE_COMMAND}" -E copy "${file_path}" "${out_file}"
        VERBATIM
    )

    get_filename_component(target "${file_path}" NAME)
    string(REGEX REPLACE "[ .-]" "_" target "${target}")
    string(PREPEND target "copy_")
    while(TARGET "${target}")
        string(APPEND target "_")
    endwhile()
    add_custom_target("${target}" ALL
        DEPENDS "${out_file}"
    )
endfunction()
