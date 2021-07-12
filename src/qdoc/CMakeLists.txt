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
        access.h
        aggregate.cpp aggregate.h
        atom.cpp atom.h
        clangcodeparser.cpp clangcodeparser.h
        classnode.cpp classnode.h
        codechunk.cpp codechunk.h
        codemarker.cpp codemarker.h
        codeparser.cpp codeparser.h
        collectionnode.cpp collectionnode.h
        config.cpp config.h
        cppcodemarker.cpp cppcodemarker.h
        cppcodeparser.cpp cppcodeparser.h
        doc.cpp doc.h
        docbookgenerator.cpp docbookgenerator.h
        docparser.cpp docparser.h
        docprivate.cpp docprivate.h
        docutilities.h
        editdistance.cpp editdistance.h
        enumitem.h
        enumnode.cpp enumnode.h
        examplenode.h
        externalpagenode.cpp externalpagenode.h
        functionnode.cpp functionnode.h
        generator.cpp generator.h
        headernode.cpp headernode.h
        helpprojectwriter.cpp helpprojectwriter.h
        htmlgenerator.cpp htmlgenerator.h
        importrec.h
        jscodemarker.cpp jscodemarker.h
        location.cpp location.h
        macro.h
        main.cpp
        manifestwriter.cpp manifestwriter.h
        namespacenode.cpp namespacenode.h
        node.cpp node.h
        openedlist.cpp openedlist.h
        pagenode.cpp pagenode.h
        parameters.cpp parameters.h
        propertynode.cpp propertynode.h
        proxynode.cpp proxynode.h
        puredocparser.cpp puredocparser.h
        qdoccommandlineparser.cpp qdoccommandlineparser.h
        qdocdatabase.cpp qdocdatabase.h
        qdocindexfiles.cpp qdocindexfiles.h
        qmlcodemarker.cpp qmlcodemarker.h
        qmlcodeparser.cpp qmlcodeparser.h
        qmlmarkupvisitor.cpp qmlmarkupvisitor.h
        qmlpropertynode.cpp qmlpropertynode.h
        qmltypenode.cpp qmltypenode.h
        qmlvisitor.cpp qmlvisitor.h
        quoter.cpp quoter.h
        relatedclass.cpp relatedclass.h
        sections.cpp sections.h
        sharedcommentnode.cpp sharedcommentnode.h
        singleton.h
        tagfilewriter.cpp tagfilewriter.h
        text.cpp text.h
        tokenizer.cpp tokenizer.h
        topic.h
        tree.cpp tree.h
        typedefnode.cpp typedefnode.h
        usingclause.cpp usingclause.h
        utilities.cpp utilities.h
        variablenode.cpp variablenode.h
        webxmlgenerator.cpp webxmlgenerator.h
        xmlgenerator.cpp xmlgenerator.h
    LIBRARIES # special case
        WrapLibClang::WrapLibClang # special case
    DEFINES
        #(CLANG_RESOURCE_DIR=\"/clang//include\") # special case remove
        CLANG_RESOURCE_DIR=${QT_LIBCLANG_RESOURCE_DIR} # special case
        QDOC2_COMPAT
)

#### Keys ignored in scope 1:.:.:qdoc.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Documentation Compiler"
# TR_EXCLUDE = "$$PWD/*"
# _OPTION = "host_build"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:qdoc.pro:NOT force_bootstrap:
# _REQUIREMENTS = "qtConfig(xmlstreamwriter)"

qt_internal_extend_target(${target_name} CONDITION TARGET Qt::QmlDevToolsPrivate
    PUBLIC_LIBRARIES
        Qt::QmlDevToolsPrivate
)

qt_internal_extend_target(${target_name} CONDITION NOT TARGET Qt::QmlDevToolsPrivate
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

