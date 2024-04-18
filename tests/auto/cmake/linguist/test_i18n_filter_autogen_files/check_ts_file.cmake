# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

file(READ "${TS_FILE}" ts_file_content)

set(expected_strings
    "<source>Hello from app1!</source>"
    "<source>Hello from app2!</source>"
)
foreach(needle IN LISTS expected_strings)
    string(FIND "${ts_file_content}" "${needle}" pos)
    if(pos EQUAL "-1")
        message(FATAL_ERROR
            "Expected string '${needle}' was not found in '${TS_FILE}'. "
            "The file content is:\n${ts_file_content}"
        )
    endif()
endforeach()

set(forbidden_strings
    "/ui_mainwindow.h\""
)
foreach(needle IN LISTS forbidden_strings)
    string(FIND "${ts_file_content}" "${needle}" pos)
    if(NOT pos EQUAL "-1")
        message(FATAL_ERROR
            "Excluded string '${needle}' was found in '${TS_FILE}'. "
            "The file content is:\n${ts_file_content}"
        )
    endif()
endforeach()
