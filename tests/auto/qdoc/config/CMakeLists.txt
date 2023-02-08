# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_config Test:
#####################################################################

qt_internal_add_test(tst_config
    SOURCES
        ../../../../src/qdoc/config.cpp ../../../../src/qdoc/config.h
        ../../../../src/qdoc/location.cpp ../../../../src/qdoc/location.h
        ../../../../src/qdoc/qdoccommandlineparser.cpp ../../../../src/qdoc/qdoccommandlineparser.h
        ../../../../src/qdoc/utilities.cpp ../../../../src/qdoc/utilities.h
        tst_config.cpp
    INCLUDE_DIRECTORIES
        ../../../../src/qdoc
)
