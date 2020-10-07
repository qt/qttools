
!force_bootstrap {
    requires(qtConfig(xmlstreamwriter))
}

option(host_build)
QT = core
qtHaveModule(qmldevtools-private) {
    QT += qmldevtools-private
} else {
    DEFINES += QT_NO_DECLARATIVE
}

DEFINES += QT_NO_FOREACH

include($$OUT_PWD/../../src/global/qttools-config.pri)

LIBS += $$CLANG_LIBS
!contains(QMAKE_DEFAULT_INCDIRS, $$CLANG_INCLUDEPATH): INCLUDEPATH += $$CLANG_INCLUDEPATH
DEFINES += $$CLANG_DEFINES

!contains(QMAKE_DEFAULT_LIBDIRS, $$CLANG_LIBDIR):!disable_external_rpath: QMAKE_RPATHDIR += $$CLANG_LIBDIR
DEFINES += $$shell_quote(CLANG_RESOURCE_DIR=\"$${CLANG_LIBDIR}/clang/$${CLANG_VERSION}/include\")

INCLUDEPATH += $$QT_SOURCE_TREE/src/tools/qdoc \
               $$QT_SOURCE_TREE/src/tools/qdoc/qmlparser

# Increase the stack size on MSVC to 4M to avoid a stack overflow
win32-icc*|win32-msvc*:{
    QMAKE_LFLAGS += /STACK:4194304
}

HEADERS += access.h \
           aggregate.h \
           atom.h \
           clangcodeparser.h \
           classnode.h \
           codechunk.h \
           codemarker.h \
           codeparser.h \
           collectionnode.h \
           config.h \
           cppcodemarker.h \
           cppcodeparser.h \
           doc.h \
           docbookgenerator.h \
           docparser.h \
           docprivate.h \
           docutilities.h \
           editdistance.h \
           enumitem.h \
           enumnode.h \
           examplenode.h \
           externalpagenode.h \
           functionnode.h \
           generator.h \
           headernode.h \
           helpprojectwriter.h \
           htmlgenerator.h \
           importrec.h \
           location.h \
           macro.h \
           manifestwriter.h \
           namespacenode.h \
           node.h \
           openedlist.h \
           pagenode.h \
           parameters.h \
           propertynode.h \
           proxynode.h \
           puredocparser.h \
           qdoccommandlineparser.h \
           qdocdatabase.h \
           qdocindexfiles.h \
           qmltypenode.h \
           qmlpropertynode.h \
           quoter.h \
           relatedclass.h \
           sections.h \
           sharedcommentnode.h \
           singleton.h \
           tagfilewriter.h \
           text.h \
           tokenizer.h \
           topic.h \
           tree.h \
           typedefnode.h \
           usingclause.h \
           utilities.h \
           variablenode.h \
           webxmlgenerator.h \
           xmlgenerator.h

SOURCES += aggregate.cpp \
           atom.cpp \
           clangcodeparser.cpp \
           classnode.cpp \
           codechunk.cpp \
           codemarker.cpp \
           codeparser.cpp \
           collectionnode.cpp \
           config.cpp \
           cppcodemarker.cpp \
           cppcodeparser.cpp \
           doc.cpp \
           docbookgenerator.cpp \
           docparser.cpp \
           docprivate.cpp \
           editdistance.cpp \
           enumnode.cpp \
           externalpagenode.cpp \
           functionnode.cpp \
           generator.cpp \
           headernode.cpp \
           helpprojectwriter.cpp \
           htmlgenerator.cpp \
           location.cpp \
           main.cpp \
           manifestwriter.cpp \
           namespacenode.cpp \
           node.cpp \
           openedlist.cpp \
           pagenode.cpp \
           parameters.cpp \
           propertynode.cpp \
           proxynode.cpp \
           puredocparser.cpp \
           qdoccommandlineparser.cpp \
           qdocdatabase.cpp \
           qdocindexfiles.cpp \
           qmltypenode.cpp \
           qmlpropertynode.cpp \
           quoter.cpp \
           relatedclass.cpp \
           sections.cpp \
           sharedcommentnode.cpp \
           tagfilewriter.cpp \
           text.cpp \
           tokenizer.cpp \
           tree.cpp \
           typedefnode.cpp \
           usingclause.cpp \
           utilities.cpp \
           variablenode.cpp \
           webxmlgenerator.cpp \
           xmlgenerator.cpp

### QML/JS Parser ###

HEADERS += jscodemarker.h \
            qmlcodemarker.h \
            qmlcodeparser.h \
            qmlmarkupvisitor.h \
            qmlvisitor.h

SOURCES += jscodemarker.cpp \
            qmlcodemarker.cpp \
            qmlcodeparser.cpp \
            qmlmarkupvisitor.cpp \
            qmlvisitor.cpp

### Documentation for qdoc ###

qtPrepareTool(QDOC, qdoc)
qtPrepareTool(QHELPGENERATOR, qhelpgenerator)

QMAKE_DOCS = $$PWD/doc/config/qdoc.qdocconf

QMAKE_TARGET_DESCRIPTION = "Qt Documentation Compiler"
load(qt_tool)

TR_EXCLUDE += $$PWD/*
