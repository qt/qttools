# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    message(WARNING "qdoc will not be built in this configuration.")
    return()
endif()

if (MINGW)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY _qt_skip_separate_debug_info ON)
endif()

# Template generator support (experimental)
option(QT_QDOC_ENABLE_TEMPLATE_GENERATOR "Enable experimental template-based generator" OFF)

if(QT_QDOC_ENABLE_TEMPLATE_GENERATOR)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
    qt_find_package(nlohmann_json MODULE REQUIRED PROVIDED_TARGETS nlohmann_json::nlohmann_json)
    qt_find_package_extend_sbom(TARGETS nlohmann_json::nlohmann_json
        LICENSE_EXPRESSION "MIT"
        SUPPLIER "Niels Lohmann"
        DOWNLOAD_LOCATION "https://github.com/nlohmann/json/releases"
        COPYRIGHTS
            "Copyright (c) 2013-2022 Niels Lohmann"
    )
    qt_find_package(Inja MODULE REQUIRED PROVIDED_TARGETS Inja::inja)
    qt_find_package_extend_sbom(TARGETS Inja::inja
        LICENSE_EXPRESSION "MIT"
        SUPPLIER "Lars Berscheid"
        DOWNLOAD_LOCATION "https://github.com/pantor/inja/releases"
        COPYRIGHTS
            "Copyright (c) 2018 Lars Berscheid"
    )
endif()


#############
## QDocLib ##
#############
# QDocLib is an internal static library used by the qdoc executable.
# It is NOT installed, NOT exported, and NOT intended for external use.
#
# This library is under active development. Its API may change without
# notice between versions. DO NOT depend on it from outside qttools.
#
# QDocLib contains components that:
# - Have no dependencies on QDoc's core infrastructure.
# - Can be instantiated and tested independently.
# - Follow clean interface/implementation separation.
set(qdoclib_sources
    src/qdoc/boundaries/filesystem/directorypath.cpp
    src/qdoc/boundaries/filesystem/filepath.cpp
    src/qdoc/boundaries/filesystem/resolvedfile.cpp
    src/qdoc/idocumentwriter.h
    src/qdoc/ioutputproducer.h
    src/qdoc/outputproducerregistry.cpp
)

# IR components are only available with template generator
if(QT_QDOC_ENABLE_TEMPLATE_GENERATOR)
    list(APPEND qdoclib_sources
        src/qdoc/ir/contentblock.cpp
        src/qdoc/ir/document.cpp
        src/qdoc/ir/link.cpp
    )
endif()

qt_add_library(QDocLib STATIC ${qdoclib_sources})

target_include_directories(QDocLib
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(QDocLib
    PUBLIC
        Qt::Core
)

#####################################################################
## qdoc Tool:
#####################################################################

qt_get_tool_target_name(target_name qdoc)

set(qdoc_sources
    src/qdoc/aggregate.cpp
    src/qdoc/atom.cpp
    src/qdoc/clangcodeparser.cpp
    src/qdoc/classnode.cpp
    src/qdoc/codechunk.cpp
    src/qdoc/codemarker.cpp
    src/qdoc/codeparser.cpp
    src/qdoc/collectionnode.cpp
    src/qdoc/comparisoncategory.h
    src/qdoc/config.cpp
    src/qdoc/cppcodemarker.cpp
    src/qdoc/cppcodeparser.cpp
    src/qdoc/doc.cpp
    src/qdoc/docbookgenerator.cpp
    src/qdoc/docparser.cpp
    src/qdoc/docprivate.cpp
    src/qdoc/editdistance.cpp
    src/qdoc/enumnode.cpp
    src/qdoc/externalpagenode.cpp
    src/qdoc/filedocumentwriter.cpp
    src/qdoc/filesystem/fileresolver.cpp
    src/qdoc/functionnode.cpp
    src/qdoc/generator.cpp
    src/qdoc/genustypes.h
    src/qdoc/headernode.cpp
    src/qdoc/helpprojectwriter.cpp
    src/qdoc/htmlgenerator.cpp
    src/qdoc/inclusionfilter.h
    src/qdoc/inclusionflags.h
    src/qdoc/inclusionpolicy.h
    src/qdoc/location.cpp
    src/qdoc/main.cpp
    src/qdoc/manifestwriter.cpp
    src/qdoc/namespacenode.cpp
    src/qdoc/nativeenum.cpp
    src/qdoc/node.cpp
    src/qdoc/nodecontext.h
    src/qdoc/openedlist.cpp
    src/qdoc/outputcontext.cpp
    src/qdoc/outputdirectory.cpp
    src/qdoc/pagenode.cpp
    src/qdoc/parameters.cpp
    src/qdoc/parsererror.cpp
    src/qdoc/propertynode.cpp
    src/qdoc/proxynode.cpp
    src/qdoc/puredocparser.cpp
    src/qdoc/qdoccommandlineparser.cpp
    src/qdoc/qdocdatabase.cpp
    src/qdoc/qdocindexfiles.cpp
    src/qdoc/qmlcodemarker.cpp
    src/qdoc/qmlcodeparser.cpp
    src/qdoc/qmlmarkupvisitor.cpp
    src/qdoc/qmlpropertyarguments.cpp
    src/qdoc/qmlpropertynode.cpp
    src/qdoc/qmltypenode.cpp
    src/qdoc/qmlvisitor.cpp
    src/qdoc/quoter.cpp
    src/qdoc/relatedclass.cpp
    src/qdoc/sections.cpp
    src/qdoc/sharedcommentnode.cpp
    src/qdoc/status.h
    src/qdoc/tagfilewriter.cpp
    src/qdoc/text.cpp
    src/qdoc/tocwriter.cpp
    src/qdoc/tokenizer.cpp
    src/qdoc/tree.cpp
    src/qdoc/typedefnode.cpp
    src/qdoc/utilities.cpp
    src/qdoc/variablenode.cpp
    src/qdoc/webxmlgenerator.cpp
    src/qdoc/xmlgenerator.cpp
)

if(QT_QDOC_ENABLE_TEMPLATE_GENERATOR)
    list(APPEND qdoc_sources
        src/qdoc/documentationtraverser.cpp
        src/qdoc/injabridge.cpp
        src/qdoc/ir/builder.cpp
        src/qdoc/templategenerator.cpp
    )
endif()

qt_internal_add_tool(${target_name}
    REQUIRED_FOR_DOCS
    TARGET_DESCRIPTION "Qt Documentation Compiler"
    TOOLS_TARGET Tools
    USER_FACING
    SOURCES
        ${qdoc_sources}
    NO_UNITY_BUILD_SOURCES
        src/qdoc/qmlmarkupvisitor.cpp # redefinition of 'samp'/'slt' (from codemarker.cpp)
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}/src
    LIBRARIES
        QDocLib
        Qt::QmlPrivate
        WrapLibClang::WrapLibClang
    DEFINES
        # To provide the ability to workaround version-specific Clang issues.
        # A re-export of (LLVM|CLANG)_VERSION_MAJOR done in WrapLibClang.cmake
        LIBCLANG_VERSION_MAJOR=${QT_LIB_CLANG_VERSION_MAJOR}
)

# Template generator dependencies
if(QT_QDOC_ENABLE_TEMPLATE_GENERATOR)
    qt_internal_extend_target(${target_name}
        LIBRARIES
            Inja::inja
            nlohmann_json::nlohmann_json
        DEFINES
            QDOC_TEMPLATE_GENERATOR_ENABLED
    )
endif()

# Embedded templates
if(QT_QDOC_ENABLE_TEMPLATE_GENERATOR)
    set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/src/qdoc/templates/page.html"
        PROPERTIES QT_RESOURCE_ALIAS "templates/page.html"
    )
    set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/src/qdoc/templates/page.md"
        PROPERTIES QT_RESOURCE_ALIAS "templates/page.md"
    )

    set(qdoc_template_files
        "${CMAKE_CURRENT_LIST_DIR}/src/qdoc/templates/page.html"
        "${CMAKE_CURRENT_LIST_DIR}/src/qdoc/templates/page.md"
    )

    qt_internal_add_resource(${target_name} "qdoc_templates"
        PREFIX
            "/qdoc"
        FILES
            ${qdoc_template_files}
    )
endif()

qt_internal_return_unless_building_tools()

# If libclangTooling.a is not built with -fPIE enabled we cannot link it to qdoc.
# TODO: Re-enable PIE once clang is built with PIE in provisioning.
set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE FALSE)

qt_internal_extend_target(${target_name} CONDITION (WIN32 AND ICC) OR MSVC
    LINK_OPTIONS
        "/STACK:4194304"
)
qt_internal_add_docs(${target_name}
    doc/config/qdoc.qdocconf
)

if(QT_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Test coverage support
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/QDocCoverage.cmake)
