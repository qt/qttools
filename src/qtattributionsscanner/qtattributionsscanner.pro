include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(qtattributionsscanner))

option(host_build)
CONFIG += console

SOURCES += \
    jsongenerator.cpp \
    main.cpp \
    packagefilter.cpp \
    qdocgenerator.cpp \
    scanner.cpp

HEADERS += \
    jsongenerator.h \
    logging.h \
    package.h \
    packagefilter.h \
    qdocgenerator.h \
    scanner.h

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

QMAKE_TARGET_DESCRIPTION = "Qt Source Code Attribution Scanner"
load(qt_tool)

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

cmake_qattributionsscanner_config_file.input = $$PWD/Qt5AttributionsScannerTools.cmake.in
cmake_qattributionsscanner_config_version_file.input = $$[QT_HOST_DATA/src]/mkspecs/features/data/cmake/Qt5ConfigVersion.cmake.in
CMAKE_PACKAGE_VERSION = $$MODULE_VERSION
cmake_qattributionsscanner_config_file.output = $$MODULE_BASE_OUTDIR/lib/cmake/Qt5AttributionsScannerTools/Qt5AttributionsScannerToolsConfig.cmake
cmake_qattributionsscanner_config_version_file.output = $$MODULE_BASE_OUTDIR/lib/cmake/Qt5AttributionsScannerTools/Qt5AttributionsScannerToolsConfigVersion.cmake
QMAKE_SUBSTITUTES += cmake_qattributionsscanner_config_file cmake_qattributionsscanner_config_version_file

cmake_qattributionsscanner_tools_files.files += $$cmake_qattributionsscanner_config_file.output $$cmake_qattributionsscanner_config_version_file.output
cmake_qattributionsscanner_tools_files.path = $$[QT_INSTALL_LIBS]/cmake/Qt5AttributionsScannerTools
cmake_qattributionsscanner_tools_files.CONFIG = no_check_exist
INSTALLS += cmake_qattributionsscanner_tools_files
