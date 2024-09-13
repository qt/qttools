# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#### Inputs



#### Libraries



#### Tests

# libclang
# special case begin
# Even though Qt builds with qmake and libclang 6.0, it fails with CMake.
# Presumably because 6.0 ClangConfig.cmake files are not good enough?
# In any case explicitly request a minimum version of 8.x for now, otherwise
# building with CMake will fail at compilation time.
qt_find_package(WrapLibClang 8 PROVIDED_TARGETS WrapLibClang::WrapLibClang)
# special case end

if(TARGET WrapLibClang::WrapLibClang)
    set(TEST_libclang "ON" CACHE BOOL "Required libclang version found." FORCE)
endif()



#### Features

# Check whether the sqlite plugin is available.
set(sqlite_plugin_available FALSE)
if(NOT QT_CONFIGURE_RUNNING)
    if(TARGET ${QT_CMAKE_EXPORT_NAMESPACE}::Sql)
        get_target_property(sql_plugins ${QT_CMAKE_EXPORT_NAMESPACE}::Sql QT_PLUGINS)
        if(QSQLiteDriverPlugin IN_LIST sql_plugins)
            set(sqlite_plugin_available TRUE)
        endif()
    endif()
endif()

qt_feature("assistant" PRIVATE
    LABEL "Qt Assistant"
    PURPOSE "Qt Assistant is a tool for viewing on-line documentation in Qt help file format."
    CONDITION TARGET Qt::Widgets AND TARGET Qt::Network AND QT_FEATURE_png AND QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton AND (sqlite_plugin_available OR QT_BUILD_SHARED_LIBS)
)
qt_feature("clang" PRIVATE
    LABEL "libclang found"
    CONDITION TEST_libclang
)
qt_feature("clang-rtti" PRIVATE
    LABEL "libclang has RTTI support"
    CONDITION QT_FEATURE_clang AND LLVM_ENABLE_RTTI
)
qt_feature("qdoc" PRIVATE
    LABEL "QDoc"
    PURPOSE "QDoc is Qt's documentation generator for C++ and QML projects."
    CONDITION TARGET Qt::QmlPrivate AND QT_FEATURE_clang AND QT_FEATURE_commandlineparser AND QT_FEATURE_thread AND QT_LIB_CLANG_VERSION VERSION_GREATER_EQUAL QDOC_MINIMUM_CLANG_VERSION
)
qt_feature("clangcpp" PRIVATE
    LABEL "Clang-based lupdate parser"
    CONDITION QT_FEATURE_clang_rtti AND (NOT MSVC OR MSVC_VERSION LESS "1939" OR QT_LIB_CLANG_VERSION_MAJOR GREATER_EQUAL "16")
)
qt_feature("designer" PRIVATE
    LABEL "Qt Widgets Designer"
    PURPOSE "Qt Widgets Designer is the Qt tool for designing and building graphical user interfaces (GUIs) with Qt Widgets. You can compose and customize your windows or dialogs in a what-you-see-is-what-you-get (WYSIWYG) manner, and test them using different styles and resolutions."
    CONDITION TARGET Qt::Widgets AND TARGET Qt::Network AND QT_FEATURE_png AND QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton
)
qt_feature("distancefieldgenerator" PRIVATE
    LABEL "Qt Distance Field Generator"
    PURPOSE "The Qt Distance Field Generator tool can be used to pregenerate the font cache in order to optimize startup performance."
    CONDITION TARGET Qt::Widgets AND QT_FEATURE_png AND QT_FEATURE_thread AND QT_FEATURE_toolbutton AND TARGET Qt::Quick
)
qt_feature("kmap2qmap" PRIVATE
    LABEL "kmap2qmap"
    PURPOSE "kmap2qmap is a tool to generate keymaps for use on Embedded Linux. The source files have to be in standard Linux kmap format that is e.g. understood by the kernel's loadkeys command."
)
qt_feature("linguist" PRIVATE
    LABEL "Qt Linguist"
    PURPOSE "Qt Linguist can be used by translator to translate text in Qt applications."
)
qt_feature("pixeltool" PRIVATE
    LABEL "pixeltool"
    PURPOSE "The Qt Pixel Zooming Tool is a graphical application that magnifies the screen around the mouse pointer so you can look more closely at individual pixels."
    CONDITION TARGET Qt::Widgets AND QT_FEATURE_png AND QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton
)
qt_feature("qdbus" PRIVATE
    LABEL "qdbus"
    PURPOSE "qdbus is a communication interface for Qt-based applications."
    CONDITION TARGET Qt::DBus
)
qt_feature("qev" PRIVATE
    LABEL "qev"
    PURPOSE "qev allows introspection of incoming events for a QWidget, similar to the X11 xev tool."
)
qt_feature("qtattributionsscanner" PRIVATE
    LABEL "Qt Attributions Scanner"
    PURPOSE "Qt Attributions Scanner generates attribution documents for third-party code in Qt."
    CONDITION QT_FEATURE_commandlineparser
)
qt_feature("qtdiag" PRIVATE
    LABEL "qtdiag"
    PURPOSE "qtdiag outputs information about the Qt installation it was built with."
    CONDITION QT_FEATURE_commandlineparser AND TARGET Qt::Gui AND NOT ANDROID AND NOT QNX AND NOT UIKIT AND NOT WASM
)
qt_feature("qtplugininfo" PRIVATE
    LABEL "qtplugininfo"
    PURPOSE "qtplugininfo dumps metadata about Qt plugins in JSON format."
    CONDITION QT_FEATURE_commandlineparser AND QT_FEATURE_library AND (android_app OR NOT ANDROID)
)
qt_configure_add_summary_section(NAME "Qt Tools")
qt_configure_add_summary_entry(ARGS "assistant")
qt_configure_add_summary_entry(ARGS "clang")
qt_configure_add_summary_entry(ARGS "clangcpp")
qt_configure_add_summary_entry(ARGS "designer")
qt_configure_add_summary_entry(ARGS "distancefieldgenerator")
#qt_configure_add_summary_entry(ARGS "kmap2qmap")
qt_configure_add_summary_entry(ARGS "linguist")
qt_configure_add_summary_entry(ARGS "pixeltool")
qt_configure_add_summary_entry(ARGS "qdbus")
qt_configure_add_summary_entry(ARGS "qdoc")
#qt_configure_add_summary_entry(ARGS "qev")
qt_configure_add_summary_entry(ARGS "qtattributionsscanner")
qt_configure_add_summary_entry(ARGS "qtdiag")
qt_configure_add_summary_entry(ARGS "qtplugininfo")
qt_configure_end_summary_section() # end of "Qt Tools" section
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "
QDoc will not be compiled, probably because clang's C and C++ libraries could not be located.
This means that you cannot build the Qt documentation. You may need to set the CMake variables
CMAKE_PREFIX_PATH or LLVM_INSTALL_DIR to the location of your llvm installation.
Other than clang's libraries, you may need to install another package, such as clang itself, to
provide the ClangConfig.cmake file needed to detect your libraries. Once this file is in place,
the configure script may be able to detect your system-installed libraries without further
environment variables.

NOTE: You WILL also need to set the FEATURE_clang CMake variable to ON to re-try finding the
dependencies.

On macOS, you can use Homebrew's llvm package.
Run `brew install llvm` and then configure with

 `configure LLVM_INSTALL_DIR=/opt/homebrew/opt/llvm FEATURE_clang=ON` for macOS arm64
or
 `configure LLVM_INSTALL_DIR=/usr/local/opt/llvm FEATURE_clang_ON` for macOS x86_64.

On Linux, you can try installing the clang package from your distribution's package manager.
On Debian / Ubuntu run `sudo apt install libclang-dev`.
On Fedora / RHEL run `sudo dnf install clang-devel`.
On ArchLinux run `sudo pacman -S clang llvm`.

Alternatively, you can use the prebuilt binaries hosted by Qt.
These let you link LLVM/Clang libraries statically, but only support Release builds on Windows.
https://download.qt.io/development_releases/prebuilt/libclang/qt/

After installing, reconfigure with

 `/qt_src/configure FEATURE_clang=ON` for a top-level build that includes the qttools repo
or
 `/qt_build/qt-configure-module /path/to/qttools FEATURE_clang=ON` for a per-submodule build
"
    CONDITION NOT QT_FEATURE_clang
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "QDoc will not be compiled because the QmlPrivate library wasn't found."
    CONDITION NOT TARGET Qt::QmlPrivate
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "QDoc cannot be compiled without Qt's commandline parser or thread features."
    CONDITION NOT QT_FEATURE_commandlineparser OR NOT QT_FEATURE_thread
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "QDoc will not be compiled because it requires libclang ${QDOC_MINIMUM_CLANG_VERSION} or newer."
    CONDITION QT_LIB_CLANG_VERSION VERSION_LESS QDOC_MINIMUM_CLANG_VERSION
)

set(clangcpp_warn_msg "")
if(QT_FEATURE_clang AND NOT QT_FEATURE_clang_rtti)
    string(APPEND clangcpp_warn_msg
        "LLVM was found, but it was not built with RTTI support.
Consider using a different prebuilt LLVM package or building LLVM with RTTI support to
enable the Clang-based lupdate parser.
Configuring LLVM with RTTI support can be done by setting the LLVM_ENABLE_RTTI CMake
variable to ON. See https://llvm.org/docs/CMake.html#building-llvm-with-cmake
and https://llvm.org/docs/CMake.html#llvm-related-variables for details.
"
    )
endif()
string(APPEND clangcpp_warn_msg
    "The Clang-based lupdate parser will not be available. "
    "Suitable LLVM and Clang C++ libraries have not been found. "
    "You will need to set the FEATURE_clangcpp CMake variable to ON to re-evaluate this check."
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "${clangcpp_warn_msg}"
    CONDITION NOT QT_FEATURE_clangcpp
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "Qt Assistant will not be compiled because it requires Qt Network."
    CONDITION NOT QT_FEATURE_assistant AND NOT TARGET Qt::Network
)

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "Qt Designer will not be compiled because it requires Qt Network."
    CONDITION NOT QT_FEATURE_designer AND NOT TARGET Qt::Network
)
