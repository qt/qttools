# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_config Test:
#####################################################################

qt_internal_add_test(tst_config
    SOURCES
        ../../../../src/qdoc/qdoc/config.cpp ../../../../src/qdoc/qdoc/config.h
        ../../../../src/qdoc/qdoc/location.cpp ../../../../src/qdoc/qdoc/location.h
        ../../../../src/qdoc/qdoc/qdoccommandlineparser.cpp ../../../../src/qdoc/qdoc/qdoccommandlineparser.h
        ../../../../src/qdoc/qdoc/utilities.cpp ../../../../src/qdoc/qdoc/utilities.h
        tst_config.cpp
    INCLUDE_DIRECTORIES
        ../../../../src/qdoc/qdoc
)
