# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    message(WARNING "qdoc will not be built in this configuration.")
    return()
endif()

if (MINGW)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY _qt_skip_separate_debug_info ON)
endif()


#####################################################################
## qdoc Tool:
#####################################################################

qt_get_tool_target_name(target_name qdoc)
qt_internal_add_tool(${target_name}
    REQUIRED_FOR_DOCS
    TARGET_DESCRIPTION "Qt Documentation Compiler"
    TOOLS_TARGET Tools
    USER_FACING
    SOURCES
        src/qdoc/aggregate.cpp
        src/qdoc/atom.cpp
        src/qdoc/boundaries/filesystem/directorypath.cpp
        src/qdoc/boundaries/filesystem/filepath.cpp
        src/qdoc/boundaries/filesystem/resolvedfile.cpp
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
        src/qdoc/filesystem/fileresolver.cpp
        src/qdoc/functionnode.cpp
        src/qdoc/generator.cpp
        src/qdoc/genustypes.h
        src/qdoc/headernode.cpp
        src/qdoc/helpprojectwriter.cpp
        src/qdoc/htmlgenerator.cpp
        src/qdoc/location.cpp
        src/qdoc/main.cpp
        src/qdoc/manifestwriter.cpp
        src/qdoc/namespacenode.cpp
        src/qdoc/nativeenum.cpp
        src/qdoc/node.cpp
        src/qdoc/openedlist.cpp
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
        src/qdoc/tagfilewriter.cpp
        src/qdoc/text.cpp
        src/qdoc/tokenizer.cpp
        src/qdoc/tree.cpp
        src/qdoc/typedefnode.cpp
        src/qdoc/utilities.cpp
        src/qdoc/variablenode.cpp
        src/qdoc/webxmlgenerator.cpp
        src/qdoc/xmlgenerator.cpp
    NO_UNITY_BUILD_SOURCES
        src/qdoc/qmlmarkupvisitor.cpp # redefinition of 'samp'/'slt' (from codemarker.cpp)
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}/src
    LIBRARIES
        Qt::QmlPrivate
        WrapLibClang::WrapLibClang
    DEFINES
        # To provide the ability to workaround version-specific Clang issues.
        # A re-export of (LLVM|CLANG)_VERSION_MAJOR done in WrapLibClang.cmake
        LIBCLANG_VERSION_MAJOR=${QT_LIB_CLANG_VERSION_MAJOR}
)
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
