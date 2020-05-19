
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

include($$OUT_PWD/qtqdoc-config.pri)

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

HEADERS += atom.h \
           clangcodeparser.h \
           codechunk.h \
           codemarker.h \
           codeparser.h \
           config.h \
           cppcodemarker.h \
           cppcodeparser.h \
           doc.h \
           docbookgenerator.h \
           editdistance.h \
           generator.h \
           helpprojectwriter.h \
           htmlgenerator.h \
           location.h \
           loggingcategory.h \
           node.h \
           openedlist.h \
           parameters.h \
           puredocparser.h \
           qdocdatabase.h \
           qdoctagfiles.h \
           qdocindexfiles.h \
           quoter.h \
           sections.h \
           separator.h \
           text.h \
           tokenizer.h \
           tree.h \
           xmlgenerator.h \
           webxmlgenerator.h \
           qdoccommandlineparser.h \
           utilities.h

SOURCES += atom.cpp \
           clangcodeparser.cpp \
           codechunk.cpp \
           codemarker.cpp \
           codeparser.cpp \
           config.cpp \
           cppcodemarker.cpp \
           cppcodeparser.cpp \
           doc.cpp \
           docbookgenerator.cpp \
           editdistance.cpp \
           generator.cpp \
           helpprojectwriter.cpp \
           htmlgenerator.cpp \
           location.cpp \
           main.cpp \
           node.cpp \
           openedlist.cpp \
           parameters.cpp \
           puredocparser.cpp \
           qdocdatabase.cpp \
           qdoctagfiles.cpp \
           qdocindexfiles.cpp \
           quoter.cpp \
           sections.cpp \
           separator.cpp \
           text.cpp \
           tokenizer.cpp \
           tree.cpp \
           xmlgenerator.cpp \
           yyindent.cpp \
           webxmlgenerator.cpp \
           qdoccommandlineparser.cpp \
           utilities.cpp

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

load(cmake_functions)

CMAKE_INSTALL_LIBS_DIR = $$cmakeTargetPath($$[QT_INSTALL_LIBS])
contains(CMAKE_INSTALL_LIBS_DIR, ^(/usr)?/lib(64)?.*): CMAKE_USR_MOVE_WORKAROUND = $$CMAKE_INSTALL_LIBS_DIR

CMAKE_LIB_DIR = $$cmakeRelativePath($$[QT_INSTALL_LIBS], $$[QT_INSTALL_PREFIX])
!contains(CMAKE_LIB_DIR,"^\\.\\./.*") {
    CMAKE_RELATIVE_INSTALL_LIBS_DIR = $$cmakeRelativePath($$[QT_INSTALL_PREFIX], $$[QT_INSTALL_LIBS])
    # We need to go up another two levels because the CMake files are
    # installed in $${CMAKE_LIB_DIR}/cmake/Qt5$${CMAKE_MODULE_NAME}
    CMAKE_RELATIVE_INSTALL_DIR = "$${CMAKE_RELATIVE_INSTALL_LIBS_DIR}../../"
} else {
    CMAKE_LIB_DIR_IS_ABSOLUTE = True
}

CMAKE_BIN_DIR = $$cmakeRelativePath($$[QT_HOST_BINS], $$[QT_INSTALL_PREFIX])
contains(CMAKE_BIN_DIR, "^\\.\\./.*") {
    CMAKE_BIN_DIR = $$[QT_HOST_BINS]/
    CMAKE_BIN_DIR_IS_ABSOLUTE = True
}

load(qt_build_paths)

equals(QMAKE_HOST.os, Windows): CMAKE_BIN_SUFFIX = ".exe"

cmake_qdoc_config_file.input = $$PWD/Qt5DocToolsConfig.cmake.in
cmake_qdoc_config_version_file.input = $$[QT_HOST_DATA/src]/mkspecs/features/data/cmake/Qt5ConfigVersion.cmake.in
CMAKE_PACKAGE_VERSION = $$MODULE_VERSION
cmake_qdoc_config_file.output = $$MODULE_BASE_OUTDIR/lib/cmake/Qt5DocTools/Qt5DocToolsConfig.cmake
cmake_qdoc_config_version_file.output = $$MODULE_BASE_OUTDIR/lib/cmake/Qt5DocTools/Qt5DocToolsConfigVersion.cmake
QMAKE_SUBSTITUTES += cmake_qdoc_config_file cmake_qdoc_config_version_file

cmake_qdoc_tools_files.files += $$cmake_qdoc_config_file.output $$cmake_qdoc_config_version_file.output
cmake_qdoc_tools_files.path = $$[QT_INSTALL_LIBS]/cmake/Qt5DocTools
cmake_qdoc_tools_files.CONFIG = no_check_exist
INSTALLS += cmake_qdoc_tools_files
