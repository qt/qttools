# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Include QDoc-specific configuration early (needed for feature definitions)
include(${CMAKE_CURRENT_LIST_DIR}/src/qdoc/cmake/QDocConfiguration.cmake)

#### Tests

qt_find_package(WrapLibClang 8 PROVIDED_TARGETS WrapLibClang::WrapLibClang)

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
    CONDITION sqlite_plugin_available OR QT_BUILD_SHARED_LIBS
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
qt_feature("fullqthelp" PUBLIC
    LABEL "fullqthelp"
    PURPOSE "Builds Help with Gui and Widget dependency."
    CONDITION (TARGET Qt::Widgets) AND (TARGET Qt::Network) AND QT_FEATURE_png AND
        QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton
)
qt_configure_add_summary_section(NAME "Qt Tools")
qt_configure_add_summary_entry(ARGS "assistant")
qt_configure_add_summary_entry(ARGS "clang")
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

# Generate QDoc-specific warning messages
if(NOT QT_CONFIGURE_RUNNING)
    include(${CMAKE_CURRENT_LIST_DIR}/src/qdoc/cmake/QDocConfigureMessages.cmake)
    qdoc_generate_clang_warning_message(QDOC_CLANG_WARNING)
    qdoc_generate_qmlprivate_warning_message(QDOC_QMLPRIVATE_WARNING)
    qdoc_generate_missing_features_warning_message(QDOC_MISSING_FEATURES_WARNING)
    qdoc_generate_clang_version_warning_message(QDOC_CLANG_VERSION_WARNING)
endif()

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "${QDOC_CLANG_WARNING}"
    CONDITION NOT QT_FEATURE_clang
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "${QDOC_QMLPRIVATE_WARNING}"
    CONDITION NOT TARGET Qt::QmlPrivate
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "${QDOC_MISSING_FEATURES_WARNING}"
    CONDITION NOT QT_FEATURE_commandlineparser OR NOT QT_FEATURE_thread
)
qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "${QDOC_CLANG_VERSION_WARNING}"
    CONDITION QT_LIB_CLANG_VERSION VERSION_LESS QDOC_MINIMUM_CLANG_VERSION
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
