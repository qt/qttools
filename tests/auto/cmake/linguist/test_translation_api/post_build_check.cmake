# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Simply check, whether the lupdate calls are reporting errors.
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build . --target update_translations
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE process_result
)
if(NOT process_result EQUAL "0")
    message(FATAL_ERROR "Command error: ${process_result}")
endif()
