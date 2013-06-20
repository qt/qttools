TEMPLATE = subdirs
SUBDIRS  = \
    lrelease \
    lupdate \
    lconvert
!no-png:qtHaveModule(widgets): SUBDIRS += linguist

qtNomakeTools( \
    linguist \
)

win32:CMAKE_BIN_SUFFIX = ".exe"

CMAKE_QT_INSTALL_PREFIX_ESCAPED = "^$$re_escape($$[QT_INSTALL_PREFIX])/"
win32:CMAKE_QT_INSTALL_PREFIX_ESCAPED ~= s,\\\\,/

CMAKE_LIB_DIR = $$replace($$list($$[QT_INSTALL_LIBS]), \\\\, /)/
contains(CMAKE_LIB_DIR, "$${CMAKE_QT_INSTALL_PREFIX_ESCAPED}.*") {
    CMAKE_LIB_DIR = $$replace(CMAKE_LIB_DIR, "$$CMAKE_QT_INSTALL_PREFIX_ESCAPED", )
    CMAKE_RELATIVE_INSTALL_DIR = $$replace(CMAKE_LIB_DIR, "[^/]+", ..)
    # We need to go up another two levels because the CMake files are
    # installed in $${CMAKE_LIB_DIR}/cmake/Qt5$${CMAKE_MODULE_NAME}
    CMAKE_RELATIVE_INSTALL_DIR = "$${CMAKE_RELATIVE_INSTALL_DIR}/../../"
}

CMAKE_BIN_DIR = $$replace($$list($$[QT_INSTALL_BINS]), \\\\, /)/
contains(CMAKE_BIN_DIR, "$${CMAKE_QT_INSTALL_PREFIX_ESCAPED}.*") {
    CMAKE_BIN_DIR = $$replace(CMAKE_BIN_DIR, "$$CMAKE_QT_INSTALL_PREFIX_ESCAPED", )
} else {
    CMAKE_BIN_DIR_IS_ABSOLUTE = True
}

load(qt_build_paths)

cmake_linguist_config_file.input = $$PWD/Qt5LinguistToolsConfig.cmake.in
cmake_linguist_config_version_file.input = $$[QT_HOST_DATA/get]/mkspecs/features/data/cmake/Qt5ConfigVersion.cmake.in
cmake_linguist_macros_file.input = $$PWD/Qt5LinguistToolsMacros.cmake
CMAKE_PACKAGE_VERSION = $$MODULE_VERSION
cmake_linguist_config_file.output = $$MODULE_BASE_OUTDIR/lib/cmake/Qt5LinguistTools/Qt5LinguistToolsConfig.cmake
cmake_linguist_config_version_file.output = $$MODULE_BASE_OUTDIR/lib/cmake/Qt5LinguistTools/Qt5LinguistToolsConfigVersion.cmake
cmake_linguist_macros_file.output = $$MODULE_BASE_OUTDIR/lib/cmake/Qt5LinguistTools/Qt5LinguistToolsMacros.cmake
cmake_linguist_macros_file.CONFIG = verbatim
QMAKE_SUBSTITUTES += cmake_linguist_config_file cmake_linguist_config_version_file cmake_linguist_macros_file

cmake_linguist_tools_files.files += $$cmake_linguist_config_file.output $$cmake_linguist_config_version_file.output $$cmake_linguist_macros_file.output
cmake_linguist_tools_files.path = $$[QT_INSTALL_LIBS]/cmake/Qt5LinguistTools
cmake_linguist_tools_files.CONFIG = no_check_exists
INSTALLS += cmake_linguist_tools_files
