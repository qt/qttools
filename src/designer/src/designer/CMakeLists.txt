# Generated from designer.pro.

#####################################################################
## designer Binary:
#####################################################################

qt_add_executable(designer
    GUI
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
    PUBLIC_LIBRARIES
        Qt::Designer
        Qt::DesignerComponents
        Qt::Gui
        Qt::Network
        Qt::Widgets
        Qt::Xml
    ENABLE_AUTOGEN_TOOLS
        uic
    PRECOMPILED_HEADER
        "qdesigner_pch.h"
)

# Resources:
set(designer_resource_files
    "images/designer.png"
)

qt_add_resource(designer "designer"
    PREFIX
        "/qt-project.org/designer"
    FILES
        ${designer_resource_files}
)
set_source_files_properties("../../../shared/qttoolbardialog/images/back.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/back.png"
)
set_source_files_properties("../../../shared/qttoolbardialog/images/down.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/down.png"
)
set_source_files_properties("../../../shared/qttoolbardialog/images/forward.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/forward.png"
)
set_source_files_properties("../../../shared/qttoolbardialog/images/minus.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/minus.png"
)
set_source_files_properties("../../../shared/qttoolbardialog/images/plus.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/plus.png"
)
set_source_files_properties("../../../shared/qttoolbardialog/images/up.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/up.png"
)
set(qttoolbardialog_resource_files
    "images/back.png"
    "images/down.png"
    "images/forward.png"
    "images/minus.png"
    "images/plus.png"
    "images/up.png"
)

qt_add_resource(designer "qttoolbardialog"
    PREFIX
        "/qt-project.org/qttoolbardialog"
    BASE
        "../../../shared/qttoolbardialog"
    FILES
        ${qttoolbardialog_resource_files}
)


#### Keys ignored in scope 1:.:.:designer.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt" "Designer"

## Scopes:
#####################################################################

qt_extend_target(designer CONDITION TARGET Qt::PrintSupport
    PUBLIC_LIBRARIES
        Qt::PrintSupport
)

qt_extend_target(designer CONDITION QT_CONFIG___contains___static
    DEFINES
        QT_DESIGNER_STATIC
)

#### Keys ignored in scope 4:.:.:designer.pro:WIN32:
# RC_ICONS = "designer.ico"
# VERSION = "$${QT_VERSION}.0"

#### Keys ignored in scope 5:.:.:designer.pro:else:
# VERSION = "$${QT_VERSION}"

#### Keys ignored in scope 6:.:.:designer.pro:APPLE:
# FILETYPES.files = "uifile.icns"
# FILETYPES.path = "Contents/Resources"
# ICON = "designer.icns"
# QMAKE_BUNDLE_DATA = "FILETYPES"
# QMAKE_INFO_PLIST = "Info_mac.plist"

qt_extend_target(designer CONDITION UNIX AND NOT APPLE_OSX AND NOT HAIKU
    PUBLIC_LIBRARIES
        m
)
qt_add_docs(designer
    doc/qtdesigner.qdocconf
)

