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
