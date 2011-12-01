TEMPLATE = subdirs
SUBDIRS  = \
    lrelease \
    lupdate \
    lconvert
!no-png:!contains(QT_CONFIG, no-gui):SUBDIRS += linguist

win32:CMAKE_BIN_SUFFIX = ".exe"
CMAKE_RELATIVE_INSTALL_DIR = "../../../"
cmake_linguist_config_file.input = $$PWD/Qt5LinguistToolsConfig.cmake.in
cmake_linguist_config_file.output = $$OUT_PWD/Qt5LinguistToolsConfig.cmake
QMAKE_SUBSTITUTES += cmake_linguist_config_file

cmake_linguist_tools_files.files += $$cmake_linguist_config_file.output $$PWD/Qt5LinguistToolsMacros.cmake
cmake_linguist_tools_files.path = $$[QT_INSTALL_LIBS]/cmake/Qt5LinguistTools
cmake_linguist_tools_files.CONFIG = no_check_exists
INSTALLS += cmake_linguist_tools_files
