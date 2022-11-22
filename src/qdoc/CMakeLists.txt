# Generated from qdoc.pro.

# special case begin
if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    message(WARNING "qdoc will not be built in this configuration.")
    return()
endif()

if (MINGW)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY _qt_skip_separate_debug_info ON)
endif()

# special case end

#####################################################################
## qdoc Tool:
#####################################################################

qt_get_tool_target_name(target_name qdoc)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Documentation Compiler"
    TOOLS_TARGET Tools # special case
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
        jscodemarker.cpp
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
        usingclause.cpp
        utilities.cpp
        variablenode.cpp
        webxmlgenerator.cpp
        xmlgenerator.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_LIST_DIR}
    LIBRARIES # special case
        WrapLibClang::WrapLibClang # special case
    DEFINES
        #(CLANG_RESOURCE_DIR=\"/clang//include\") # special case remove
        CLANG_RESOURCE_DIR=${QT_LIBCLANG_RESOURCE_DIR} # special case
        # To provide the ability to workaround version-specific Clang issues.
        # A re-export of (LLVM|CLANG)_VERSION_MAJOR done in WrapLibClang.cmake
        LIBCLANG_VERSION_MAJOR=${QT_LIB_CLANG_VERSION_MAJOR}
        QDOC2_COMPAT
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:qdoc.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Documentation Compiler"
# TR_EXCLUDE = "$$PWD/*"
# _OPTION = "host_build"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:qdoc.pro:NOT force_bootstrap:
# _REQUIREMENTS = "qtConfig(xmlstreamwriter)"

qt_internal_extend_target(${target_name} CONDITION TARGET Qt::QmlPrivate
    LIBRARIES
        Qt::QmlPrivate
)

qt_internal_extend_target(${target_name} CONDITION NOT TARGET Qt::QmlPrivate
    DEFINES
        QT_NO_DECLARATIVE
)

#### Keys ignored in scope 6:.:.:qdoc.pro:NOT QMAKE_DEFAULT_LIBDIRS___contains____ss_CLANG_LIBDIR AND NOT disable_external_rpath:
# QMAKE_RPATHDIR = "$$CLANG_LIBDIR"

qt_internal_extend_target(${target_name} CONDITION (WIN32 AND ICC) OR MSVC
    LINK_OPTIONS
        "/STACK:4194304"
)
qt_internal_add_docs(${target_name}
    doc/config/qdoc.qdocconf
)
