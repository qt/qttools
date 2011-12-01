TEMPLATE = subdirs
SUBDIRS = qdbus qdbusxml2cpp qdbuscpp2xml
!contains(QT_CONFIG, no-gui): SUBDIRS += qdbusviewer

win32:CMAKE_BIN_SUFFIX = ".exe"
CMAKE_RELATIVE_INSTALL_DIR = "../../../"
cmake_dbus_config_file.input = $$PWD/Qt5DBusToolsConfig.cmake.in
cmake_dbus_config_file.output = $$OUT_PWD/Qt5DBusToolsConfig.cmake
QMAKE_SUBSTITUTES += cmake_dbus_config_file

cmake_dbus_tools_files.files += $$cmake_dbus_config_file.output $$PWD/Qt5DBusToolsMacros.cmake
cmake_dbus_tools_files.path = $$[QT_INSTALL_LIBS]/cmake/Qt5DBusTools
cmake_dbus_tools_files.CONFIG = no_check_exists
INSTALLS += cmake_dbus_tools_files
