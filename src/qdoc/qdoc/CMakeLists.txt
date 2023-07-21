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
    TARGET_DESCRIPTION "Qt Documentation Compiler"
    TOOLS_TARGET Tools
    USER_FACING
    SOURCES
        aggregate.cpp
        atom.cpp
        boundaries/filesystem/directorypath.cpp
        boundaries/filesystem/filepath.cpp
        boundaries/filesystem/resolvedfile.cpp
        clangcodeparser.cpp
        classnode.cpp
        codechunk.cpp
        codemarker.cpp
        codeparser.cpp
        collectionnode.cpp
        config.cpp
        cppcodemarker.cpp
        cppcodeparser.cpp
        doc.cpp
        docbookgenerator.cpp
        docparser.cpp
        docprivate.cpp
        editdistance.cpp
        enumnode.cpp
        externalpagenode.cpp
        filesystem/fileresolver.cpp
        functionnode.cpp
        generator.cpp
        headernode.cpp
        helpprojectwriter.cpp
        htmlgenerator.cpp
        location.cpp
        main.cpp
        manifestwriter.cpp
        namespacenode.cpp
        node.cpp
        openedlist.cpp
        pagenode.cpp
        parameters.cpp
        propertynode.cpp
        proxynode.cpp
        puredocparser.cpp
        qdoccommandlineparser.cpp
        qdocdatabase.cpp
        qdocindexfiles.cpp
        qmlcodemarker.cpp
        qmlcodeparser.cpp
        qmlmarkupvisitor.cpp
        qmlpropertynode.cpp
        qmltypenode.cpp
        qmlvisitor.cpp
        quoter.cpp
        relatedclass.cpp
        sections.cpp
        sharedcommentnode.cpp
        tagfilewriter.cpp
        text.cpp
        tokenizer.cpp
        tree.cpp
        typedefnode.cpp
        utilities.cpp
        variablenode.cpp
        webxmlgenerator.cpp
        xmlgenerator.cpp
    NO_UNITY_BUILD_SOURCES
        qmlmarkupvisitor.cpp # redefinition of 'samp'/'slt' (from codemarker.cpp)
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}
    LIBRARIES
        Qt::QmlPrivate
        WrapLibClang::WrapLibClang
    DEFINES
        #(CLANG_RESOURCE_DIR=\"/clang//include\") # special case remove
        CLANG_RESOURCE_DIR=${QT_LIBCLANG_RESOURCE_DIR}
        # To provide the ability to workaround version-specific Clang issues.
        # A re-export of (LLVM|CLANG)_VERSION_MAJOR done in WrapLibClang.cmake
        LIBCLANG_VERSION_MAJOR=${QT_LIB_CLANG_VERSION_MAJOR}
        QDOC2_COMPAT
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
