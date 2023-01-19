# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## designer App:
#####################################################################

qt_internal_add_app(designer
    SOURCES
        ../../../shared/fontpanel/fontpanel.cpp ../../../shared/fontpanel/fontpanel.h
        ../../../shared/qttoolbardialog/qttoolbardialog.cpp ../../../shared/qttoolbardialog/qttoolbardialog.h ../../../shared/qttoolbardialog/qttoolbardialog.ui
        appfontdialog.cpp appfontdialog.h
        assistantclient.cpp assistantclient.h
        designer_enums.h
        main.cpp
        mainwindow.cpp mainwindow.h
        newform.cpp newform.h
        preferencesdialog.cpp preferencesdialog.h preferencesdialog.ui
        qdesigner.cpp qdesigner.h
        qdesigner_actions.cpp qdesigner_actions.h
        qdesigner_appearanceoptions.cpp qdesigner_appearanceoptions.h qdesigner_appearanceoptions.ui
        qdesigner_formwindow.cpp qdesigner_formwindow.h
        qdesigner_server.cpp qdesigner_server.h
        qdesigner_settings.cpp qdesigner_settings.h
        qdesigner_toolwindow.cpp qdesigner_toolwindow.h
        qdesigner_workbench.cpp qdesigner_workbench.h
        saveformastemplate.cpp saveformastemplate.h saveformastemplate.ui
        versiondialog.cpp versiondialog.h
    INCLUDE_DIRECTORIES
        ../../../shared/fontpanel
        ../../../shared/qttoolbardialog
        ../lib/extension
        ../lib/sdk
        ../lib/shared
        extra
    LIBRARIES
        Qt::CorePrivate
        Qt::DesignerComponentsPrivate
        Qt::DesignerPrivate
        Qt::Gui
        Qt::Network
        Qt::Widgets
        Qt::Xml
    ENABLE_AUTOGEN_TOOLS
        uic
    PRECOMPILED_HEADER
        "qdesigner_pch.h"
)

# Due to QTBUG-110369, don't add designer as dependency to External Project examples.
qt_internal_skip_dependency_for_examples(designer)

# Resources:
set(designer_resource_files
    "images/designer.png"
)

qt_internal_add_resource(designer "designer"
    PREFIX
        "/qt-project.org/designer"
    FILES
        ${designer_resource_files}
)
set(qttoolbardialog_resource_files
    "../../../shared/qttoolbardialog/images/back.png"
    "../../../shared/qttoolbardialog/images/down.png"
    "../../../shared/qttoolbardialog/images/forward.png"
    "../../../shared/qttoolbardialog/images/minus.png"
    "../../../shared/qttoolbardialog/images/plus.png"
    "../../../shared/qttoolbardialog/images/up.png"
)

qt_internal_add_resource(designer "qttoolbardialog"
    PREFIX
        "/qt-project.org/qttoolbardialog"
    BASE
        "../../../shared/qttoolbardialog"
    FILES
        ${qttoolbardialog_resource_files}
)

set_target_properties(designer PROPERTIES
    QT_TARGET_DESCRIPTION "Qt Designer"
)

## Scopes:
#####################################################################

qt_internal_extend_target(designer CONDITION TARGET Qt::PrintSupport
    PUBLIC_LIBRARIES
        Qt::PrintSupport
)

qt_internal_extend_target(designer CONDITION NOT QT_BUILD_SHARED_LIBS
    DEFINES
        QT_DESIGNER_STATIC
)

if(WIN32)
    set_target_properties(designer PROPERTIES
        QT_TARGET_RC_ICONS "${CMAKE_CURRENT_SOURCE_DIR}/designer.ico"
    )
endif()

if(WIN32)
    set_target_properties(designer PROPERTIES
        QT_TARGET_VERSION "${PROJECT_VERSION}.0"
    )
endif()

if(UNIX)
    set_target_properties(designer PROPERTIES
        QT_TARGET_VERSION "${PROJECT_VERSION}"
    )
endif()

if(APPLE)
    set_target_properties(designer PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info_mac.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "designer.icns"
        OUTPUT_NAME "Designer"
    )
    set_source_files_properties(designer.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(designer PRIVATE
        designer.icns
    )
    # special case end
    # Set values to be replaced in the custom Info_mac.plist.
    # Also package the uifile.icns.
    set(ICON "designer.icns")
    set(EXECUTABLE "Designer")
    set_source_files_properties(uifile.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(designer PRIVATE uifile.icns)
    # special case end
endif()

# FILETYPES.files = "uifile.icns"
# FILETYPES.path = "Contents/Resources"
# QMAKE_BUNDLE_DATA = "FILETYPES"

qt_internal_extend_target(designer CONDITION UNIX AND NOT HAIKU AND NOT MACOS
    PUBLIC_LIBRARIES
        m
)
qt_internal_add_docs(designer
    doc/qtdesigner.qdocconf
)
