# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## assistant App:
#####################################################################

qt_internal_add_app(assistant
    SOURCES
        ../../shared/fontpanel/fontpanel.cpp ../../shared/fontpanel/fontpanel_p.h
        ../shared/collectionconfiguration.cpp ../shared/collectionconfiguration.h
        aboutdialog.cpp aboutdialog.h
        bookmarkdialog.cpp bookmarkdialog.h bookmarkdialog.ui
        bookmarkfiltermodel.cpp bookmarkfiltermodel.h
        bookmarkitem.cpp bookmarkitem.h
        bookmarkmanager.cpp bookmarkmanager.h
        bookmarkmanagerwidget.cpp bookmarkmanagerwidget.h bookmarkmanagerwidget.ui
        bookmarkmodel.cpp bookmarkmodel.h
        bookmarkwidget.ui
        centralwidget.cpp centralwidget.h
        cmdlineparser.cpp cmdlineparser.h
        contentwindow.cpp contentwindow.h
        findwidget.cpp findwidget.h
        globalactions.cpp globalactions.h
        helpbrowsersupport.cpp helpbrowsersupport.h
        helpdocsettings.cpp helpdocsettings.h
        helpdocsettingswidget.cpp helpdocsettingswidget.h helpdocsettingswidget.ui
        helpenginewrapper.cpp helpenginewrapper.h
        helpviewer.cpp helpviewer.h helpviewerimpl.cpp helpviewerimpl.h helpviewerimpl_p.h
        indexwindow.cpp indexwindow.h
        main.cpp
        mainwindow.cpp mainwindow.h
        openpagesmanager.cpp openpagesmanager.h
        openpagesmodel.cpp openpagesmodel.h
        openpagesswitcher.cpp openpagesswitcher.h
        openpageswidget.cpp openpageswidget.h
        preferencesdialog.cpp preferencesdialog.h preferencesdialog.ui
        qtdocinstaller.cpp qtdocinstaller.h
        remotecontrol.cpp remotecontrol.h
        searchwidget.cpp searchwidget.h
        topicchooser.cpp topicchooser.h topicchooser.ui
        tracer.h
        xbelsupport.cpp xbelsupport.h
    INCLUDE_DIRECTORIES
        ../../shared/fontpanel
    LIBRARIES
        Qt::Gui
        Qt::Help
        Qt::Network
        Qt::Sql
        Qt::Widgets
        Qt::PrintSupport
        qlitehtml
    ATTRIBUTION_FILE_DIR_PATHS
        ../qlitehtml/src/3rdparty
)

qt_add_ui(assistant SOURCES
    bookmarkdialog.ui
    bookmarkmanagerwidget.ui
    bookmarkwidget.ui
    helpdocsettingswidget.ui
    preferencesdialog.ui
    topicchooser.ui
)

# Resources:
set(assistant_images_resource_files
    "images/assistant-128.png"
    "images/assistant.png"
    "images/bookmark.png"
    "images/closebutton.png"
    "images/darkclosebutton.png"
    "images/mac/book.png"
    "images/mac/closetab.png"
    "images/mac/editcopy.png"
    "images/mac/find.png"
    "images/mac/home.png"
    "images/mac/next.png"
    "images/mac/previous.png"
    "images/mac/print.png"
    "images/mac/resetzoom.png"
    "images/mac/synctoc.png"
    "images/mac/zoomin.png"
    "images/mac/zoomout.png"
    "images/win/book.png"
    "images/win/closetab.png"
    "images/win/editcopy.png"
    "images/win/find.png"
    "images/win/home.png"
    "images/win/next.png"
    "images/win/previous.png"
    "images/win/print.png"
    "images/win/resetzoom.png"
    "images/win/synctoc.png"
    "images/win/zoomin.png"
    "images/win/zoomout.png"
    "images/wrap.png"
)

qt_internal_add_resource(assistant "assistant_images"
    PREFIX
        "/qt-project.org/assistant"
    FILES
        ${assistant_images_resource_files}
)

set_target_properties(assistant PROPERTIES
    QT_TARGET_DESCRIPTION "Qt Assistant"
)

## Scopes:
#####################################################################

qt_internal_extend_target(assistant CONDITION TARGET Qt::PrintSupport
    PUBLIC_LIBRARIES
        Qt::PrintSupport
)

qt_internal_extend_target(assistant CONDITION BROWSER___equals___qtwebkit
    SOURCES
        helpviewerimpl_qwv.cpp
    DEFINES
        BROWSER_QTWEBKIT
    PUBLIC_LIBRARIES
        Qt::WebKitWidgets
)

qt_internal_extend_target(assistant CONDITION NOT BROWSER___equals___qtwebkit
    SOURCES
        helpviewerimpl_qtb.cpp
    DEFINES
        BROWSER_QTEXTBROWSER
)

if(WIN32)
    set_target_properties(assistant PROPERTIES
        QT_TARGET_RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/assistant.ico"
    )
endif()

if(WIN32)
    set_target_properties(assistant PROPERTIES
        QT_TARGET_VERSION "${PROJECT_VERSION}.0"
    )
endif()

qt_internal_extend_target(assistant CONDITION WIN32
    SOURCES
        stdinlistener_win.cpp stdinlistener_win.h
    PUBLIC_LIBRARIES
        shell32
)

if(UNIX)
    set_target_properties(assistant PROPERTIES
        QT_TARGET_VERSION "${PROJECT_VERSION}"
    )
endif()

qt_internal_extend_target(assistant CONDITION UNIX
    SOURCES
        stdinlistener.cpp stdinlistener.h
)

if(QT_INSTALL_XDG_DESKTOP_ENTRIES)
    if(UNIX AND NOT APPLE)
        qt_path_join(xdg_install_dir ${QT_INSTALL_DIR} ${CMAKE_INSTALL_DATAROOTDIR})

        qt_install(FILES assistant.desktop
            DESTINATION "${xdg_install_dir}/applications"
        )
        qt_install(FILES assistant.metainfo.xml
            RENAME io.qt.Assistant.metainfo.xml
            DESTINATION "${xdg_install_dir}/metainfo"
        )
        qt_install(FILES images/assistant-128.png
            RENAME assistant.png
            DESTINATION "${xdg_install_dir}/icons/hicolor/128x128/apps"
        )
    endif()
endif()

if(APPLE)
    set_target_properties(assistant PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info_mac.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "assistant.icns"
        OUTPUT_NAME "Assistant"
    )
    set_source_files_properties(assistant.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(assistant PRIVATE
        assistant.icns
    )
    # Set values to be replaced in the custom Info_mac.plist.
    set(ICON "assistant.icns")
    set(EXECUTABLE "Assistant")
endif()
qt_internal_add_docs(assistant
    doc/qtassistant.qdocconf
)

