# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(ts_file "test_i18n_exclusion_de.ts")
file(READ "${ts_file}" ts_file_content)

set(expected_strings
    "<source>Hello from app1!</source>"
    "<source>Hello from MyObject1!</source>"
)
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
    "<source>Hello from MyObject2!</source>"
    "<source>Hello from test1!</source>"
    "<source>excluded1</source>"
    "<source>excluded2</source>"
    "<source>excluded3</source>"
    "<source>excluded4</source>"
    "<source>excluded5</source>"
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
