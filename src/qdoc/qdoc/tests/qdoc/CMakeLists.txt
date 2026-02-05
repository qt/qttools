# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_QDoc LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(QT_QDOC_ENABLE_TEMPLATE_GENERATOR)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../cmake")
    find_package(nlohmann_json REQUIRED)
    find_package(Inja REQUIRED)
endif()

qt_internal_add_test(tst_QDoc
    SOURCES
        main.cpp
        boundaries/filesystem/catch_filepath.cpp
        boundaries/filesystem/catch_directorypath.cpp
        filesystem/catch_fileresolver.cpp
        node/catch_genustypes.cpp
        inclusion/catch_inclusionstrategytests.cpp
        outputproducerregistry/catch_outputproducerregistry.cpp
        ../../src/qdoc/boundaries/filesystem/filepath.cpp
        ../../src/qdoc/boundaries/filesystem/directorypath.cpp
        ../../src/qdoc/boundaries/filesystem/resolvedfile.cpp
        ../../src/qdoc/filesystem/fileresolver.cpp
        ../../src/qdoc/outputproducerregistry.cpp
        ../../src/qdoc/genustypes.h
    INCLUDE_DIRECTORIES
        ../../src/
    LIBRARIES
        Qt::QDocCatchPrivate
        Qt::QDocCatchConversionsPrivate
        Qt::QDocCatchGeneratorsPrivate
)

qt_internal_extend_target(tst_QDoc
    CONDITION QT_QDOC_ENABLE_TEMPLATE_GENERATOR
    SOURCES
        injabridge/catch_injabridge.cpp
        templategenerator/catch_templategenerator.cpp
        documentir/catch_documentir.cpp
        ir/catch_linkir.cpp
        irbuilder/catch_irbuilder.cpp
        ../../src/qdoc/injabridge.cpp
        ../../src/qdoc/ir/documentir.cpp
        ../../src/qdoc/ir/linkir.cpp
        # Note: irbuilder.cpp is NOT included because it depends on Doc, Text,
        # Atom, and other classes that would pull in most of QDoc. The test
        # verifies header compilation; full integration testing is done by
        # tst_validateTemplateGeneratorOutput.
    LIBRARIES
        Inja::inja
        nlohmann_json::nlohmann_json
    DEFINES
        QDOC_TEMPLATE_GENERATOR_ENABLED
)
