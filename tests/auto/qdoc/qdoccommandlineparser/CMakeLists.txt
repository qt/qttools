# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_qdoccommandlineparser Test:
#####################################################################

qt_internal_add_test(tst_qdoccommandlineparser
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES
        ../../../../src/qdoc/qdoccommandlineparser.cpp ../../../../src/qdoc/qdoccommandlineparser.h
        ../../../../src/qdoc/utilities.cpp ../../../../src/qdoc/utilities.h
        tst_qdoccommandlineparser.cpp
    INCLUDE_DIRECTORIES
        ../../../../src/qdoc
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

