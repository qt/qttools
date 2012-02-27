TEMPLATE = subdirs
SUBDIRS = qdbus qdbusxml2cpp qdbuscpp2xml
!contains(QT_CONFIG, no-gui): SUBDIRS += qdbusviewer

win32:CMAKE_BIN_SUFFIX = ".exe"

CMAKE_QT_INSTALL_PREFIX = $$replace($$list($$[QT_INSTALL_PREFIX]), \\\\, /)/
CMAKE_QT_INSTALL_PREFIX_ESCAPED = "^$$re_escape($$CMAKE_QT_INSTALL_PREFIX)"

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

cmake_dbus_config_file.input = $$PWD/Qt5DBusToolsConfig.cmake.in
cmake_dbus_config_file.output = $$OUT_PWD/Qt5DBusToolsConfig.cmake
QMAKE_SUBSTITUTES += cmake_dbus_config_file

cmake_dbus_tools_files.files += $$cmake_dbus_config_file.output $$PWD/Qt5DBusToolsMacros.cmake
cmake_dbus_tools_files.path = $$[QT_INSTALL_LIBS]/cmake/Qt5DBusTools
cmake_dbus_tools_files.CONFIG = no_check_exists
INSTALLS += cmake_dbus_tools_files
