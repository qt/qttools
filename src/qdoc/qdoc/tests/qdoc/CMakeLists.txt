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
        parameters/catch_parameter.cpp
        ../../src/qdoc/parameter.cpp
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
        ir/catch_document.cpp
        ir/catch_contentblock.cpp
        ir/catch_link.cpp
        ir/catch_atomcontext.cpp
        ir/catch_builder.cpp
        ir/catch_contentbuilder.cpp
        ir/catch_member.cpp
        ir/catch_signaturespan.cpp
        ../../src/qdoc/injabridge.cpp
        ../../src/qdoc/textutils.cpp
        ../../src/qdoc/ir/member.cpp
        ../../src/qdoc/ir/builder.cpp
        ../../src/qdoc/ir/contentbuilder.cpp
        ../../src/qdoc/ir/contentblock.cpp
        ../../src/qdoc/ir/document.cpp
        ../../src/qdoc/ir/link.cpp
        ../../src/qdoc/ir/signaturespan.cpp
    LIBRARIES
        Inja::inja
        nlohmann_json::nlohmann_json
    DEFINES
        QDOC_TEMPLATE_GENERATOR_ENABLED
)
