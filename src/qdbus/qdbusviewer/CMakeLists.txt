# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qdbusviewer App:
#####################################################################

qt_internal_add_app(qdbusviewer
    SOURCES
        logviewer.cpp logviewer.h
        main.cpp
        mainwindow.cpp mainwindow.h
        propertydialog.cpp propertydialog.h
        qdbusmodel.cpp qdbusmodel.h
        qdbusviewer.cpp qdbusviewer.h
        servicesproxymodel.cpp servicesproxymodel.h
    LIBRARIES
        Qt::DBusPrivate
        Qt::Gui
        Qt::Widgets
        Qt::Xml
)

# Resources:
set(qdbusviewer_resource_files
    "images/qdbusviewer-128.png"
    "images/qdbusviewer.png"
)

qt_internal_add_resource(qdbusviewer "qdbusviewer"
    PREFIX
        "/qt-project.org/qdbusviewer"
    FILES
        ${qdbusviewer_resource_files}
)


## Scopes:
#####################################################################

if(QT_INSTALL_XDG_DESKTOP_ENTRIES)
    if(UNIX AND NOT APPLE)
        qt_path_join(xdg_install_dir ${QT_INSTALL_DIR} ${CMAKE_INSTALL_DATAROOTDIR})

        qt_install(FILES qdbusviewer.desktop
            DESTINATION "${xdg_install_dir}/applications"
        )
        qt_install(FILES qdbusviewer.metainfo.xml
            RENAME io.qt.qdbusviewer.metainfo.xml
            DESTINATION "${xdg_install_dir}/metainfo"
        )
        qt_install(FILES images/qdbusviewer-128.png
            RENAME qdbusviewer.png
            DESTINATION "${xdg_install_dir}/icons/hicolor/128x128/apps"
        )
    endif()
endif()

if(APPLE)
    set_target_properties(qdbusviewer PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info_mac.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "qdbusviewer.icns"
    )
    set_source_files_properties(images/qdbusviewer.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(qdbusviewer PRIVATE
        images/qdbusviewer.icns
    )
    # Set values to be replaced in the custom Info_mac.plist.
    set(ICON "qdbusviewer.icns")
    set(EXECUTABLE "qdbusviewer")
endif()

if(WIN32)
    set_target_properties(qdbusviewer PROPERTIES
        QT_TARGET_RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/images/qdbusviewer.ico"
    )
endif()
