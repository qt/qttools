# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## Designer Module:
#####################################################################

qt_internal_add_module(Designer
    PLUGIN_TYPES designer
    SOURCES
        ../../../shared/deviceskin/deviceskin.cpp ../../../shared/deviceskin/deviceskin_p.h
        ../../../shared/findwidget/abstractfindwidget.cpp ../../../shared/findwidget/abstractfindwidget_p.h
        ../../../shared/findwidget/itemviewfindwidget.cpp ../../../shared/findwidget/itemviewfindwidget_p.h
        ../../../shared/findwidget/texteditfindwidget.cpp ../../../shared/findwidget/texteditfindwidget_p.h
        ../../../shared/qtgradienteditor/qtcolorbutton.cpp ../../../shared/qtgradienteditor/qtcolorbutton_p.h
        ../../../shared/qtgradienteditor/qtcolorline.cpp ../../../shared/qtgradienteditor/qtcolorline_p.h
        ../../../shared/qtgradienteditor/qtgradientdialog.cpp ../../../shared/qtgradienteditor/qtgradientdialog_p.h
        ../../../shared/qtgradienteditor/qtgradienteditor.cpp ../../../shared/qtgradienteditor/qtgradienteditor_p.h
        ../../../shared/qtgradienteditor/qtgradientmanager.cpp ../../../shared/qtgradienteditor/qtgradientmanager_p.h
        ../../../shared/qtgradienteditor/qtgradientstopscontroller.cpp ../../../shared/qtgradienteditor/qtgradientstopscontroller_p.h
        ../../../shared/qtgradienteditor/qtgradientstopsmodel.cpp ../../../shared/qtgradienteditor/qtgradientstopsmodel_p.h
        ../../../shared/qtgradienteditor/qtgradientstopswidget.cpp ../../../shared/qtgradienteditor/qtgradientstopswidget_p.h
        ../../../shared/qtgradienteditor/qtgradientutils.cpp ../../../shared/qtgradienteditor/qtgradientutils_p.h
        ../../../shared/qtgradienteditor/qtgradientview.cpp ../../../shared/qtgradienteditor/qtgradientview_p.h
        ../../../shared/qtgradienteditor/qtgradientviewdialog.cpp ../../../shared/qtgradienteditor/qtgradientviewdialog_p.h
        ../../../shared/qtgradienteditor/qtgradientwidget.cpp ../../../shared/qtgradienteditor/qtgradientwidget_p.h
        extension/default_extensionfactory.cpp extension/default_extensionfactory.h
        extension/extension_global.h
        extension/extension.cpp extension/extension.h
        extension/qextensionmanager.cpp extension/qextensionmanager.h
        sdk/abstractactioneditor.cpp sdk/abstractactioneditor.h
        sdk/abstractdialoggui.cpp sdk/abstractdialoggui_p.h
        sdk/abstractdnditem.h
        sdk/abstractformeditor.cpp sdk/abstractformeditor.h
        sdk/abstractformeditorplugin.cpp sdk/abstractformeditorplugin.h
        sdk/abstractformwindow.cpp sdk/abstractformwindow.h
        sdk/abstractformwindowcursor.cpp sdk/abstractformwindowcursor.h
        sdk/abstractformwindowmanager.cpp sdk/abstractformwindowmanager.h
        sdk/abstractformwindowtool.cpp sdk/abstractformwindowtool.h
        sdk/abstractintegration.cpp sdk/abstractintegration.h
        sdk/abstractintrospection.cpp sdk/abstractintrospection_p.h
        sdk/abstractlanguage.h
        sdk/abstractmetadatabase.cpp sdk/abstractmetadatabase.h
        sdk/abstractnewformwidget.cpp sdk/abstractnewformwidget.h
        sdk/abstractobjectinspector.cpp sdk/abstractobjectinspector.h
        sdk/abstractoptionspage.h
        sdk/abstractpromotioninterface.cpp sdk/abstractpromotioninterface.h
        sdk/abstractpropertyeditor.cpp sdk/abstractpropertyeditor.h
        sdk/abstractresourcebrowser.cpp sdk/abstractresourcebrowser.h
        sdk/abstractsettings.h
        sdk/abstractwidgetbox.cpp sdk/abstractwidgetbox.h
        sdk/abstractwidgetdatabase.cpp sdk/abstractwidgetdatabase.h
        sdk/abstractwidgetfactory.cpp sdk/abstractwidgetfactory.h
        sdk/container.h
        sdk/dynamicpropertysheet.h
        sdk/extrainfo.cpp sdk/extrainfo.h
        sdk/layoutdecoration.h
        sdk/membersheet.h
        sdk/propertysheet.h
        sdk/sdk_global.h
        sdk/taskmenu.cpp sdk/taskmenu.h
        shared/actioneditor.cpp shared/actioneditor_p.h
        shared/actionprovider_p.h
        shared/actionrepository.cpp shared/actionrepository_p.h
        shared/codedialog.cpp shared/codedialog_p.h
        shared/connectionedit.cpp shared/connectionedit_p.h
        shared/csshighlighter.cpp shared/csshighlighter_p.h
        shared/deviceprofile.cpp shared/deviceprofile_p.h
        shared/dialoggui.cpp shared/dialoggui_p.h
        shared/extensionfactory_p.h
        shared/formlayoutmenu.cpp shared/formlayoutmenu_p.h
        shared/formwindowbase.cpp shared/formwindowbase_p.h
        shared/grid.cpp shared/grid_p.h
        shared/gridpanel.cpp shared/gridpanel_p.h
        shared/htmlhighlighter.cpp shared/htmlhighlighter_p.h
        shared/iconloader.cpp shared/iconloader_p.h
        shared/iconselector.cpp shared/iconselector_p.h
        shared/invisible_widget.cpp shared/invisible_widget_p.h
        shared/layout.cpp shared/layout_p.h
        shared/layoutinfo.cpp shared/layoutinfo_p.h
        shared/metadatabase.cpp shared/metadatabase_p.h
        shared/morphmenu.cpp shared/morphmenu_p.h
        shared/newactiondialog.cpp shared/newactiondialog_p.h
        shared/newformwidget.cpp shared/newformwidget_p.h
        shared/orderdialog.cpp shared/orderdialog_p.h
        shared/plaintexteditor.cpp shared/plaintexteditor_p.h
        shared/plugindialog.cpp shared/plugindialog_p.h
        shared/pluginmanager.cpp shared/pluginmanager_p.h
        shared/previewconfigurationwidget.cpp shared/previewconfigurationwidget_p.h
        shared/previewmanager.cpp shared/previewmanager_p.h
        shared/promotionmodel.cpp shared/promotionmodel_p.h
        shared/promotiontaskmenu.cpp shared/promotiontaskmenu_p.h
        shared/propertylineedit.cpp shared/propertylineedit_p.h
        shared/qdesigner_command.cpp shared/qdesigner_command_p.h
        shared/qdesigner_command2.cpp shared/qdesigner_command2_p.h
        shared/qdesigner_dnditem.cpp shared/qdesigner_dnditem_p.h
        shared/qdesigner_dockwidget.cpp shared/qdesigner_dockwidget_p.h
        shared/qdesigner_formbuilder.cpp shared/qdesigner_formbuilder_p.h
        shared/qdesigner_formeditorcommand.cpp shared/qdesigner_formeditorcommand_p.h
        shared/qdesigner_formwindowcommand.cpp shared/qdesigner_formwindowcommand_p.h
        shared/qdesigner_formwindowmanager.cpp shared/qdesigner_formwindowmanager_p.h
        shared/qdesigner_introspection.cpp shared/qdesigner_introspection_p.h
        shared/qdesigner_membersheet.cpp shared/qdesigner_membersheet_p.h
        shared/qdesigner_menu.cpp shared/qdesigner_menu_p.h
        shared/qdesigner_menubar.cpp shared/qdesigner_menubar_p.h
        shared/qdesigner_objectinspector.cpp shared/qdesigner_objectinspector_p.h
        shared/qdesigner_promotion.cpp shared/qdesigner_promotion_p.h
        shared/qdesigner_promotiondialog.cpp shared/qdesigner_promotiondialog_p.h
        shared/qdesigner_propertycommand.cpp
        shared/qdesigner_propertyeditor.cpp shared/qdesigner_propertyeditor_p.h
        shared/qdesigner_propertysheet.cpp shared/qdesigner_propertysheet_p.h
        shared/qdesigner_qsettings.cpp shared/qdesigner_qsettings_p.h
        shared/qdesigner_stackedbox.cpp shared/qdesigner_stackedbox_p.h
        shared/qdesigner_tabwidget.cpp shared/qdesigner_tabwidget_p.h
        shared/qdesigner_taskmenu.cpp shared/qdesigner_taskmenu_p.h
        shared/qdesigner_toolbar.cpp shared/qdesigner_toolbar_p.h
        shared/qdesigner_toolbox.cpp shared/qdesigner_toolbox_p.h
        shared/qdesigner_utils.cpp shared/qdesigner_utils_p.h
        shared/qdesigner_widget.cpp shared/qdesigner_widget_p.h
        shared/qdesigner_widgetbox.cpp shared/qdesigner_widgetbox_p.h
        shared/qdesigner_widgetitem.cpp shared/qdesigner_widgetitem_p.h
        shared/qlayout_widget.cpp shared/qlayout_widget_p.h
        shared/qsimpleresource.cpp shared/qsimpleresource_p.h
        shared/qtresourceeditordialog.cpp shared/qtresourceeditordialog_p.h
        shared/qtresourcemodel.cpp shared/qtresourcemodel_p.h
        shared/qtresourceview.cpp shared/qtresourceview_p.h
        shared/rcc.cpp shared/rcc_p.h
        shared/richtexteditor.cpp shared/richtexteditor_p.h
        shared/selectsignaldialog.cpp shared/selectsignaldialog_p.h
        shared/shared_enums_p.h
        shared/shared_global_p.h
        shared/shared_settings.cpp shared/shared_settings_p.h
        shared/sheet_delegate.cpp shared/sheet_delegate_p.h
        shared/signalslotdialog.cpp shared/signalslotdialog_p.h
        shared/spacer_widget.cpp shared/spacer_widget_p.h
        shared/stylesheeteditor.cpp shared/stylesheeteditor_p.h
        shared/textpropertyeditor.cpp shared/textpropertyeditor_p.h
        shared/widgetdatabase.cpp shared/widgetdatabase_p.h
        shared/widgetfactory.cpp shared/widgetfactory_p.h
        shared/zoomwidget.cpp shared/zoomwidget_p.h
        uilib/uilib_global.h
        uilib/abstractformbuilder.cpp uilib/abstractformbuilder.h
        uilib/formbuilder.cpp uilib/formbuilder.h
        uilib/formbuilderextra.cpp uilib/formbuilderextra_p.h
        uilib/properties.cpp uilib/properties_p.h
        uilib/resourcebuilder.cpp uilib/resourcebuilder_p.h
        uilib/textbuilder.cpp uilib/textbuilder_p.h
        uilib/ui4.cpp uilib/ui4_p.h
        components/qdesigner_components.h
        components/qdesigner_components_global.h
    NO_UNITY_BUILD_SOURCES
        shared/qdesigner_command.cpp # redefinition of 'QMetaTypeId<QList<QWidget *>>' (from morphmenu.cpp)
                                     # and recursiveUpdate (from formwindowbase.cpp)
        uilib/abstractformbuilder.cpp # using namespace QFormInternal/redefinition of 'QMetaTypeId<QList<QWidget *>>' (from morphmenu.cpp)
        uilib/ui4.cpp # using namespace QFormInternal
    DEFINES
        QDESIGNER_EXTENSION_LIBRARY
        QDESIGNER_SDK_LIBRARY
        QDESIGNER_SHARED_LIBRARY
        QDESIGNER_UILIB_LIBRARY
        QT_DESIGNER
        QT_USE_QSTRINGBUILDER
    INCLUDE_DIRECTORIES
        ../../../shared/deviceskin
        ../../../shared/findwidget
        ../../../shared/qtgradienteditor
        extension
        sdk
        shared
        uilib
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::UiPlugin
        Qt::WidgetsPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::UiPlugin
        Qt::Widgets
        Qt::Xml
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WidgetsPrivate
    ENABLE_AUTOGEN_TOOLS
        uic
    PRECOMPILED_HEADER
        "lib_pch.h"
    NO_GENERATE_CPP_EXPORTS
)

set(ui_sources
    ../../../shared/qtgradienteditor/qtgradientdialog.ui
    ../../../shared/qtgradienteditor/qtgradienteditor.ui
    ../../../shared/qtgradienteditor/qtgradientview.ui
    ../../../shared/qtgradienteditor/qtgradientviewdialog.ui
    shared/addlinkdialog.ui
    shared/formlayoutrowdialog.ui
    shared/gridpanel.ui
    shared/previewconfigurationwidget.ui
    shared/newactiondialog.ui
    shared/newformwidget.ui
    shared/orderdialog.ui
    shared/plugindialog.ui
    shared/qtresourceeditordialog.ui
    shared/selectsignaldialog.ui
    shared/signalslotdialog.ui
)

# Work around QTBUG-95305
if(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config" AND CMAKE_CROSS_CONFIGS)
    qt6_wrap_ui(ui_sources_processed ${ui_sources})
else()
    set(ui_sources_processed ${ui_sources})
endif()
target_sources(Designer PRIVATE ${ui_sources_processed})

# Resources:
set(ClamshellPhone_resource_files
    "../../../shared/deviceskin/skins/ClamshellPhone.skin"
)

qt_internal_add_resource(Designer "ClamshellPhone"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${ClamshellPhone_resource_files}
)
set(SmartPhone2_resource_files
    "../../../shared/deviceskin/skins/SmartPhone2.skin"
)

qt_internal_add_resource(Designer "SmartPhone2"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${SmartPhone2_resource_files}
)
set(SmartPhone_resource_files
    "../../../shared/deviceskin/skins/SmartPhone.skin"
)

qt_internal_add_resource(Designer "SmartPhone"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${SmartPhone_resource_files}
)
set(SmartPhoneWithButtons_resource_files
    "../../../shared/deviceskin/skins/SmartPhoneWithButtons.skin"
)

qt_internal_add_resource(Designer "SmartPhoneWithButtons"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${SmartPhoneWithButtons_resource_files}
)
set(TouchscreenPhone_resource_files
    "../../../shared/deviceskin/skins/TouchscreenPhone.skin"
)

qt_internal_add_resource(Designer "TouchscreenPhone"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${TouchscreenPhone_resource_files}
)
set(PortableMedia_resource_files
    "../../../shared/deviceskin/skins/PortableMedia.skin"
)

qt_internal_add_resource(Designer "PortableMedia"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${PortableMedia_resource_files}
)
set(S60-QVGA-Candybar_resource_files
    "../../../shared/deviceskin/skins/S60-QVGA-Candybar.skin"
)

qt_internal_add_resource(Designer "S60-QVGA-Candybar"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${S60-QVGA-Candybar_resource_files}
)
set(S60-nHD-Touchscreen_resource_files
    "../../../shared/deviceskin/skins/S60-nHD-Touchscreen.skin"
)

qt_internal_add_resource(Designer "S60-nHD-Touchscreen"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${S60-nHD-Touchscreen_resource_files}
)
set(findwidget_resource_files
    "../../../shared/findwidget/images/mac/closetab.png"
    "../../../shared/findwidget/images/mac/next.png"
    "../../../shared/findwidget/images/mac/previous.png"
    "../../../shared/findwidget/images/mac/searchfind.png"
    "../../../shared/findwidget/images/win/closetab.png"
    "../../../shared/findwidget/images/win/next.png"
    "../../../shared/findwidget/images/win/previous.png"
    "../../../shared/findwidget/images/win/searchfind.png"
    "../../../shared/findwidget/images/wrap.png"
)

qt_internal_add_resource(Designer "findwidget"
    PREFIX
        "/qt-project.org/shared"
    BASE
        "../../../shared/findwidget"
    FILES
        ${findwidget_resource_files}
)
set(qtgradienteditor_resource_files
    "../../../shared/qtgradienteditor/images/down.png"
    "../../../shared/qtgradienteditor/images/edit.png"
    "../../../shared/qtgradienteditor/images/editdelete.png"
    "../../../shared/qtgradienteditor/images/minus.png"
    "../../../shared/qtgradienteditor/images/plus.png"
    "../../../shared/qtgradienteditor/images/spreadpad.png"
    "../../../shared/qtgradienteditor/images/spreadreflect.png"
    "../../../shared/qtgradienteditor/images/spreadrepeat.png"
    "../../../shared/qtgradienteditor/images/typeconical.png"
    "../../../shared/qtgradienteditor/images/typelinear.png"
    "../../../shared/qtgradienteditor/images/typeradial.png"
    "../../../shared/qtgradienteditor/images/up.png"
    "../../../shared/qtgradienteditor/images/zoomin.png"
    "../../../shared/qtgradienteditor/images/zoomout.png"
)

qt_internal_add_resource(Designer "qtgradienteditor"
    PREFIX
        "/qt-project.org/qtgradienteditor"
    BASE
        "../../../shared/qtgradienteditor"
    FILES
        ${qtgradienteditor_resource_files}
)
set(shared_resource_files
    "shared/defaultgradients.xml"
    "shared/icon-naming-spec.txt"
    "shared/templates/forms/240x320/Dialog_with_Buttons_Bottom.ui"
    "shared/templates/forms/240x320/Dialog_with_Buttons_Right.ui"
    "shared/templates/forms/320x240/Dialog_with_Buttons_Bottom.ui"
    "shared/templates/forms/320x240/Dialog_with_Buttons_Right.ui"
    "shared/templates/forms/480x640/Dialog_with_Buttons_Bottom.ui"
    "shared/templates/forms/480x640/Dialog_with_Buttons_Right.ui"
    "shared/templates/forms/640x480/Dialog_with_Buttons_Bottom.ui"
    "shared/templates/forms/640x480/Dialog_with_Buttons_Right.ui"
    "shared/templates/forms/Dialog_with_Buttons_Bottom.ui"
    "shared/templates/forms/Dialog_with_Buttons_Right.ui"
    "shared/templates/forms/Dialog_without_Buttons.ui"
    "shared/templates/forms/Main_Window.ui"
    "shared/templates/forms/Widget.ui"
)

qt_internal_add_resource(Designer "shared"
    PREFIX
        "/qt-project.org/designer"
    BASE
        "shared"
    FILES
        ${shared_resource_files}
)

# MODULE = "designer"

## Scopes:
#####################################################################

qt_internal_extend_target(Designer CONDITION TARGET Qt::OpenGLWidgets
    PUBLIC_LIBRARIES
        Qt::OpenGLWidgets
)

qt_internal_extend_target(Designer CONDITION NOT QT_BUILD_SHARED_LIBS
    DEFINES
        QT_DESIGNER_STATIC
    SOURCES
        ../../../shared/qtpropertybrowser/qtbuttonpropertybrowser.cpp ../../../shared/qtpropertybrowser/qtbuttonpropertybrowser_p.h
        ../../../shared/qtpropertybrowser/qteditorfactory.cpp ../../../shared/qtpropertybrowser/qteditorfactory_p.h
        ../../../shared/qtpropertybrowser/qtgroupboxpropertybrowser.cpp ../../../shared/qtpropertybrowser/qtgroupboxpropertybrowser_p.h
        ../../../shared/qtpropertybrowser/qtpropertybrowser.cpp ../../../shared/qtpropertybrowser/qtpropertybrowser_p.h
        ../../../shared/qtpropertybrowser/qtpropertybrowserutils.cpp ../../../shared/qtpropertybrowser/qtpropertybrowserutils_p.h
        ../../../shared/qtpropertybrowser/qtpropertymanager.cpp ../../../shared/qtpropertybrowser/qtpropertymanager_p.h
        ../../../shared/qtpropertybrowser/qttreepropertybrowser.cpp ../../../shared/qtpropertybrowser/qttreepropertybrowser_p.h
        ../../../shared/qtpropertybrowser/qtvariantproperty.cpp ../../../shared/qtpropertybrowser/qtvariantproperty_p.h
    INCLUDE_DIRECTORIES
        ../../../shared/qtpropertybrowser
)

if(TARGET zstd::libzstd)
    qt_internal_disable_find_package_global_promotion(zstd::libzstd)
endif()
if(TARGET zstd::libzstd_shared)
    qt_internal_disable_find_package_global_promotion(zstd::libzstd_shared)
endif()
if(TARGET zstd::libzstd_static)
    qt_internal_disable_find_package_global_promotion(zstd::libzstd_static)
endif()
if(NOT TARGET WrapZSTD::WrapZSTD)
    qt_find_package(WrapZSTD 1.3
        PROVIDED_TARGETS
            WrapZSTD::WrapZSTD
            zstd::libzstd
            zstd::libzstd_static
            zstd::libzstd_shared
    )
endif()

qt_internal_extend_target(Designer CONDITION QT_FEATURE_zstd
    LIBRARIES
        WrapZSTD::WrapZSTD
)

if(NOT QT_BUILD_SHARED_LIBS)
    # Resources:
    set(qtpropertybrowser_resource_files
        "../../../shared/qtpropertybrowser/images/cursor-arrow.png"
        "../../../shared/qtpropertybrowser/images/cursor-busy.png"
        "../../../shared/qtpropertybrowser/images/cursor-closedhand.png"
        "../../../shared/qtpropertybrowser/images/cursor-cross.png"
        "../../../shared/qtpropertybrowser/images/cursor-forbidden.png"
        "../../../shared/qtpropertybrowser/images/cursor-hand.png"
        "../../../shared/qtpropertybrowser/images/cursor-hsplit.png"
        "../../../shared/qtpropertybrowser/images/cursor-ibeam.png"
        "../../../shared/qtpropertybrowser/images/cursor-openhand.png"
        "../../../shared/qtpropertybrowser/images/cursor-sizeall.png"
        "../../../shared/qtpropertybrowser/images/cursor-sizeb.png"
        "../../../shared/qtpropertybrowser/images/cursor-sizef.png"
        "../../../shared/qtpropertybrowser/images/cursor-sizeh.png"
        "../../../shared/qtpropertybrowser/images/cursor-sizev.png"
        "../../../shared/qtpropertybrowser/images/cursor-uparrow.png"
        "../../../shared/qtpropertybrowser/images/cursor-vsplit.png"
        "../../../shared/qtpropertybrowser/images/cursor-wait.png"
        "../../../shared/qtpropertybrowser/images/cursor-whatsthis.png"
    )

    qt_internal_add_resource(Designer "qtpropertybrowser"
        PREFIX
            "/qt-project.org/qtpropertybrowser"
        BASE
            "../../../shared/qtpropertybrowser"
        FILES
            ${qtpropertybrowser_resource_files}
    )
endif()

qt_internal_extend_target(Designer CONDITION QT_BUILD_SHARED_LIBS
    SOURCES
        ../../../shared/qtpropertybrowser/qtpropertybrowserutils.cpp ../../../shared/qtpropertybrowser/qtpropertybrowserutils_p.h
    INCLUDE_DIRECTORIES
        ../../../shared/qtpropertybrowser
)

qt_internal_extend_target(Designer CONDITION QT_FEATURE_opengl
    LIBRARIES
        Qt::OpenGL
)

# UiPlugin module generates deprecated header files for Designer.
qt_internal_add_sync_header_dependencies(Designer UiPlugin)
