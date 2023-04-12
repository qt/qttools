# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_qdoccommandlineparser Test:
#####################################################################

qt_internal_add_test(tst_qdoccommandlineparser
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES
        ${QDOC_SOURCE_DIRECTORY}/qdoccommandlineparser.cpp
        ${QDOC_SOURCE_DIRECTORY}/utilities.cpp
        tst_qdoccommandlineparser.cpp
    INCLUDE_DIRECTORIES
        ${QDOC_INCLUDE_DIRECTORY}
)

# Resources:
set(tst_qdoccommandlineparser_resource_files
    "tst_arguments.txt"
)

qt_internal_add_resource(tst_qdoccommandlineparser "tst_qdoccommandlineparser"
    PREFIX
        "/"
    FILES
        ${tst_qdoccommandlineparser_resource_files}
)

