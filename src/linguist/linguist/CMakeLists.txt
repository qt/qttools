# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## linguist App:
#####################################################################

qt_internal_add_app(linguist
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/simtexth.cpp ../shared/simtexth.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        ../shared/xmlparser.cpp ../shared/xmlparser.h
        batchtranslation.ui
        batchtranslationdialog.cpp batchtranslationdialog.h
        errorsview.cpp errorsview.h
        finddialog.cpp finddialog.h finddialog.ui
        uiformpreviewview.cpp uiformpreviewview.h
        qmlformpreviewview.h qmlformpreviewview.cpp
        globals.cpp globals.h
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        messageeditor.cpp messageeditor.h
        messageeditorwidgets.cpp messageeditorwidgets.h
        messagehighlighter.cpp messagehighlighter.h
        messagemodel.cpp messagemodel.h
        phrase.cpp phrase.h
        phrasebookbox.cpp phrasebookbox.h phrasebookbox.ui
        phrasemodel.cpp phrasemodel.h
        phraseview.cpp phraseview.h
        recentfiles.cpp recentfiles.h
        sourcecodeview.cpp sourcecodeview.h
        statistics.cpp statistics.h statistics.ui
        translatedialog.cpp translatedialog.h translatedialog.ui
        translationsettings.ui
        translationsettingsdialog.cpp translationsettingsdialog.h
    DEFINES
        QFORMINTERNAL_NAMESPACE
        QT_KEYWORDS
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::UiToolsPrivate
        Qt::Widgets
        Qt::QuickWidgets
        Qt::QuickLayouts
)

qt_internal_extend_target(linguist CONDITION QT_FEATURE_printsupport
    SOURCES
        printout.cpp printout.h
    LIBRARIES
        Qt::PrintSupport
)

set(available_plugins "")

set(plugin_targets
    Qt::qtquickcontrols2plugin
    Qt::qtquick2plugin
    Qt::qtquickdialogsplugin
    Qt::qtquickcontrols2universalstyleplugin
    Qt::qtquickcontrols2basicstyleplugin
    Qt::qtquickcontrols2fluentwinui3styleplugin
    Qt::qtquickcontrols2fusionstyleplugin
    Qt::qtquickcontrols2imaginestyleplugin
    Qt::qtquickcontrols2iosstyleplugin
    Qt::qtquickcontrols2macosstyleplugin
    Qt::qtquickcontrols2materialstyleplugin
    Qt::qtquicktemplates2plugin
    Qt::qtquickcontrols2implplugin
    Qt::qtquickdialogs2quickimplplugin
    Qt::qtquickcontrols2universalstyleimplplugin
    Qt::qtquickcontrols2basicstyleimplplugin
    Qt::qtquickcontrols2fluentwinui3styleimplplugin
    Qt::qtquickcontrols2fusionstyleimplplugin
    Qt::qtquickcontrols2imaginestyleimplplugin
    Qt::qtquickcontrols2iosstyleimplplugin
    Qt::qtquickcontrols2macosstyleimplplugin
    Qt::qtquickcontrols2materialstyleimplplugin
    Qt::quickwindow
)

foreach(plugin ${plugin_targets})
    if(TARGET ${plugin})
        list(APPEND available_plugins ${plugin})
    endif()
endforeach()

if (available_plugins)
    if(BUILD_SHARED_LIBS)
        add_dependencies(linguist ${available_plugins})
    else()
        qt_internal_extend_target(linguist LIBRARIES ${available_plugins})
    endif()
endif()

qt_add_ui(linguist
    SOURCES
        batchtranslation.ui
        finddialog.ui
        mainwindow.ui
        phrasebookbox.ui
        statistics.ui
        translatedialog.ui
        translationsettings.ui
)

# Resources:
set(linguist_resource_files
    "images/appicon.png"
    "images/icons/linguist-128-32.png"
    "images/darkicons/check-ampersands.png"
    "images/darkicons/check-ending-pontuation.png"
    "images/darkicons/check-phrase-suggestions.png"
    "images/darkicons/check-place-markers.png"
    "images/darkicons/check-white-spaces.png"
    "images/darkicons/copy-general.png"
    "images/darkicons/cut.png"
    "images/darkicons/hit-help-chosen-option.png"
    "images/darkicons/library.png"
    "images/darkicons/mark-current-translation-done.png"
    "images/darkicons/mark-current-translation-done-move-to-next.png"
    "images/darkicons/minus-square-fill.png"
    "images/darkicons/next-translation-item.png"
    "images/darkicons/next-unfinished-translation-item.png"
    "images/darkicons/open-new.png"
    "images/darkicons/paste-general.png"
    "images/darkicons/plus-square-fill.png"
    "images/darkicons/previous-translation-item.png"
    "images/darkicons/previous-unfinished-translation-item.png"
    "images/darkicons/print.png"
    "images/darkicons/redo-arrow-right.png"
    "images/darkicons/save-fl-disk.png"
    "images/darkicons/search-magnifier.png"
    "images/darkicons/undo-arrow-left.png"

    "images/darkicons/check-ampersands-disabled.png"
    "images/darkicons/check-ending-pontuation-disabled.png"
    "images/darkicons/check-phrase-suggestions-disabled.png"
    "images/darkicons/check-place-markers-disabled.png"
    "images/darkicons/check-white-spaces-disabled.png"
    "images/darkicons/copy-general-disabled.png"
    "images/darkicons/cut-disabled.png"
    "images/darkicons/hit-help-chosen-option-disabled.png"
    "images/darkicons/library-disabled.png"
    "images/darkicons/mark-current-translation-done-disabled.png"
    "images/darkicons/mark-current-translation-done-move-to-next-disabled.png"
    "images/darkicons/minus-square-fill-disabled.png"
    "images/darkicons/next-translation-item-disabled.png"
    "images/darkicons/next-unfinished-translation-item-disabled.png"
    "images/darkicons/open-new-disabled.png"
    "images/darkicons/paste-general-disabled.png"
    "images/darkicons/plus-square-fill-disabled.png"
    "images/darkicons/previous-translation-item-disabled.png"
    "images/darkicons/previous-unfinished-translation-item-disabled.png"
    "images/darkicons/print-disabled.png"
    "images/darkicons/redo-arrow-right-disabled.png"
    "images/darkicons/save-fl-disk-disabled.png"
    "images/darkicons/search-magnifier-disabled.png"
    "images/darkicons/undo-arrow-left-disabled.png"

    "images/lighticons/check-ampersands.png"
    "images/lighticons/check-ending-pontuation.png"
    "images/lighticons/check-phrase-suggestions.png"
    "images/lighticons/check-place-markers.png"
    "images/lighticons/check-white-spaces.png"
    "images/lighticons/copy-general.png"
    "images/lighticons/cut.png"
    "images/lighticons/hit-help-chosen-option.png"
    "images/lighticons/library.png"
    "images/lighticons/mark-current-translation-done.png"
    "images/lighticons/mark-current-translation-done-move-to-next.png"
    "images/lighticons/minus-square-fill.png"
    "images/lighticons/next-translation-item.png"
    "images/lighticons/next-unfinished-translation-item.png"
    "images/lighticons/open-new.png"
    "images/lighticons/paste-general.png"
    "images/lighticons/plus-square-fill.png"
    "images/lighticons/previous-translation-item.png"
    "images/lighticons/previous-unfinished-translation-item.png"
    "images/lighticons/print.png"
    "images/lighticons/redo-arrow-right.png"
    "images/lighticons/save-fl-disk.png"
    "images/lighticons/search-magnifier.png"
    "images/lighticons/undo-arrow-left.png"

    "images/lighticons/check-ampersands-disabled.png"
    "images/lighticons/check-ending-pontuation-disabled.png"
    "images/lighticons/check-phrase-suggestions-disabled.png"
    "images/lighticons/check-place-markers-disabled.png"
    "images/lighticons/check-white-spaces-disabled.png"
    "images/lighticons/copy-general-disabled.png"
    "images/lighticons/cut-disabled.png"
    "images/lighticons/hit-help-chosen-option-disabled.png"
    "images/lighticons/library-disabled.png"
    "images/lighticons/mark-current-translation-done-disabled.png"
    "images/lighticons/mark-current-translation-done-move-to-next-disabled.png"
    "images/lighticons/minus-square-fill-disabled.png"
    "images/lighticons/next-translation-item-disabled.png"
    "images/lighticons/next-unfinished-translation-item-disabled.png"
    "images/lighticons/open-new-disabled.png"
    "images/lighticons/paste-general-disabled.png"
    "images/lighticons/plus-square-fill-disabled.png"
    "images/lighticons/previous-translation-item-disabled.png"
    "images/lighticons/previous-unfinished-translation-item-disabled.png"
    "images/lighticons/print-disabled.png"
    "images/lighticons/redo-arrow-right-disabled.png"
    "images/lighticons/save-fl-disk-disabled.png"
    "images/lighticons/search-magnifier-disabled.png"
    "images/lighticons/undo-arrow-left-disabled.png"

    "images/lightmarks/danger-mark.png"
    "images/lightmarks/empty-mark.png"
    "images/lightmarks/obsolete-mark.png"
    "images/lightmarks/off-mark.png"
    "images/lightmarks/on-mark.png"
    "images/lightmarks/warning-mark.png"
    "images/darkmarks/danger-mark.png"
    "images/darkmarks/empty-mark.png"
    "images/darkmarks/obsolete-mark.png"
    "images/darkmarks/off-mark.png"
    "images/darkmarks/on-mark.png"
    "images/darkmarks/warning-mark.png"
)

qt_internal_add_resource(linguist "linguist"
    PREFIX
        "/"
    FILES
        ${linguist_resource_files}
)

set_target_properties(linguist PROPERTIES
    QT_TARGET_DESCRIPTION "Qt Linguist"
)

file(GLOB phrasebooks_files "${CMAKE_CURRENT_SOURCE_DIR}/../phrasebooks/*")
set(phrasebooks_install_dir "${INSTALL_DATADIR}/phrasebooks")
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.31")
    cmake_path(SET phrasebooks_install_dir NORMALIZE "${phrasebooks_install_dir}")
endif()
qt_install(FILES ${phrasebooks_files} DESTINATION "${phrasebooks_install_dir}")

if(WIN32)
    set_target_properties(linguist PROPERTIES
        QT_TARGET_RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/linguist.ico"
    )
endif()

if(WIN32)
    set_target_properties(linguist PROPERTIES
        QT_TARGET_VERSION "${PROJECT_VERSION}.0"
    )
endif()

if(UNIX)
    set_target_properties(linguist PROPERTIES
        QT_TARGET_VERSION "${PROJECT_VERSION}"
    )
endif()

if(QT_INSTALL_XDG_DESKTOP_ENTRIES)
    if(UNIX AND NOT APPLE)
        qt_path_join(xdg_install_dir ${QT_INSTALL_DIR} ${CMAKE_INSTALL_DATAROOTDIR})

        qt_install(FILES linguist.desktop
            DESTINATION "${xdg_install_dir}/applications"
        )
        qt_install(FILES linguist.metainfo.xml
            RENAME io.qt.Linguist.metainfo.xml
            DESTINATION "${xdg_install_dir}/metainfo"
        )
        qt_install(FILES images/icons/linguist-128-32.png
            RENAME linguist.png
            DESTINATION "${xdg_install_dir}/icons/hicolor/128x128/apps"
        )
    endif()
endif()

if(APPLE)
    set_target_properties(linguist PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info_mac.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "linguist.icns"
        OUTPUT_NAME "Linguist"
    )
    set_source_files_properties(linguist.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(linguist PRIVATE
        linguist.icns
    )
else()
    set_property(SOURCE main.cpp PROPERTY SKIP_AUTOMOC ON)
endif()
qt_internal_add_docs(linguist
    doc/qtlinguist.qdocconf
)

