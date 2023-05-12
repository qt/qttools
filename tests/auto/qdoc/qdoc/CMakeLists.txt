# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_QDoc
  SOURCES
    main.cpp

    boundaries/filesystem/catch_filepath.cpp
    boundaries/filesystem/catch_directorypath.cpp
    filesystem/catch_fileresolver.cpp

    ${QDOC_SOURCE_DIRECTORY}/boundaries/filesystem/filepath.cpp
    ${QDOC_SOURCE_DIRECTORY}/boundaries/filesystem/directorypath.cpp
    ${QDOC_SOURCE_DIRECTORY}/boundaries/filesystem/resolvedfile.cpp
    ${QDOC_SOURCE_DIRECTORY}/filesystem/fileresolver.cpp
  INCLUDE_DIRECTORIES
    ${QDOC_INCLUDE_DIRECTORY}
  LIBRARIES
    Qt::QDocCatchPrivate
    Qt::QDocCatchConversionsPrivate
    Qt::QDocCatchGeneratorsPrivate
)
