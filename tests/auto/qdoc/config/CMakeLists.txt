# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_config Test:
#####################################################################

qt_internal_add_test(tst_config
    SOURCES
        ${QDOC_SOURCE_DIRECTORY}/config.cpp
        ${QDOC_SOURCE_DIRECTORY}/location.cpp
        ${QDOC_SOURCE_DIRECTORY}/qdoccommandlineparser.cpp
        ${QDOC_SOURCE_DIRECTORY}/utilities.cpp
        tst_config.cpp
    INCLUDE_DIRECTORIES
        ${QDOC_INCLUDE_DIRECTORY}
)
