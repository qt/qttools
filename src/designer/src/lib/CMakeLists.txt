# Generated from lib.pro.

#####################################################################
## Designer Module:
#####################################################################

qt_internal_add_module(Designer
    PLUGIN_TYPES designer
    SOURCES
        ../../../shared/deviceskin/deviceskin.cpp ../../../shared/deviceskin/deviceskin.h
        ../../../shared/findwidget/abstractfindwidget.cpp ../../../shared/findwidget/abstractfindwidget.h
        ../../../shared/findwidget/itemviewfindwidget.cpp ../../../shared/findwidget/itemviewfindwidget.h
        ../../../shared/findwidget/texteditfindwidget.cpp ../../../shared/findwidget/texteditfindwidget.h
        ../../../shared/qtgradienteditor/qtcolorbutton.cpp ../../../shared/qtgradienteditor/qtcolorbutton.h
        ../../../shared/qtgradienteditor/qtcolorline.cpp ../../../shared/qtgradienteditor/qtcolorline.h
        ../../../shared/qtgradienteditor/qtgradientdialog.cpp ../../../shared/qtgradienteditor/qtgradientdialog.h ../../../shared/qtgradienteditor/qtgradientdialog.ui
        ../../../shared/qtgradienteditor/qtgradienteditor.cpp ../../../shared/qtgradienteditor/qtgradienteditor.h ../../../shared/qtgradienteditor/qtgradienteditor.ui
        ../../../shared/qtgradienteditor/qtgradientmanager.cpp ../../../shared/qtgradienteditor/qtgradientmanager.h
        ../../../shared/qtgradienteditor/qtgradientstopscontroller.cpp ../../../shared/qtgradienteditor/qtgradientstopscontroller.h
        ../../../shared/qtgradienteditor/qtgradientstopsmodel.cpp ../../../shared/qtgradienteditor/qtgradientstopsmodel.h
        ../../../shared/qtgradienteditor/qtgradientstopswidget.cpp ../../../shared/qtgradienteditor/qtgradientstopswidget.h
        ../../../shared/qtgradienteditor/qtgradientutils.cpp ../../../shared/qtgradienteditor/qtgradientutils.h
        ../../../shared/qtgradienteditor/qtgradientview.cpp ../../../shared/qtgradienteditor/qtgradientview.h ../../../shared/qtgradienteditor/qtgradientview.ui
        ../../../shared/qtgradienteditor/qtgradientviewdialog.cpp ../../../shared/qtgradienteditor/qtgradientviewdialog.h ../../../shared/qtgradienteditor/qtgradientviewdialog.ui
        ../../../shared/qtgradienteditor/qtgradientwidget.cpp ../../../shared/qtgradienteditor/qtgradientwidget.h
        extension/default_extensionfactory.cpp extension/default_extensionfactory.h
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
        sdk/taskmenu.h
        shared/actioneditor.cpp shared/actioneditor_p.h
        shared/actionprovider_p.h
        shared/actionrepository.cpp shared/actionrepository_p.h
        shared/addlinkdialog.ui
        shared/codedialog.cpp shared/codedialog_p.h
        shared/connectionedit.cpp shared/connectionedit_p.h
        shared/csshighlighter.cpp shared/csshighlighter_p.h
        shared/deviceprofile.cpp shared/deviceprofile_p.h
        shared/dialoggui.cpp shared/dialoggui_p.h
        shared/extensionfactory_p.h
        shared/formlayoutmenu.cpp shared/formlayoutmenu_p.h
        shared/formlayoutrowdialog.ui
        shared/formwindowbase.cpp shared/formwindowbase_p.h
        shared/grid.cpp shared/grid_p.h
        shared/gridpanel.cpp shared/gridpanel.ui shared/gridpanel_p.h
        shared/htmlhighlighter.cpp shared/htmlhighlighter_p.h
        shared/iconloader.cpp shared/iconloader_p.h
        shared/iconselector.cpp shared/iconselector_p.h
        shared/invisible_widget.cpp shared/invisible_widget_p.h
        shared/layout.cpp shared/layout_p.h
        shared/layoutinfo.cpp shared/layoutinfo_p.h
        shared/metadatabase.cpp shared/metadatabase_p.h
        shared/morphmenu.cpp shared/morphmenu_p.h
        shared/newactiondialog.cpp shared/newactiondialog.ui shared/newactiondialog_p.h
        shared/newformwidget.cpp shared/newformwidget.ui shared/newformwidget_p.h
        shared/orderdialog.cpp shared/orderdialog.ui shared/orderdialog_p.h
        shared/plaintexteditor.cpp shared/plaintexteditor_p.h
        shared/plugindialog.cpp shared/plugindialog.ui shared/plugindialog_p.h
        shared/pluginmanager.cpp shared/pluginmanager_p.h
        shared/previewconfigurationwidget.cpp shared/previewconfigurationwidget.ui shared/previewconfigurationwidget_p.h
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
        shared/qtresourceeditordialog.cpp shared/qtresourceeditordialog.ui shared/qtresourceeditordialog_p.h
        shared/qtresourcemodel.cpp shared/qtresourcemodel_p.h
        shared/qtresourceview.cpp shared/qtresourceview_p.h
        shared/rcc.cpp shared/rcc_p.h
        shared/richtexteditor.cpp shared/richtexteditor_p.h
        shared/selectsignaldialog.cpp shared/selectsignaldialog.ui shared/selectsignaldialog_p.h
        shared/shared_enums_p.h
        shared/shared_global_p.h
        shared/shared_settings.cpp shared/shared_settings_p.h
        shared/sheet_delegate.cpp shared/sheet_delegate_p.h
        shared/signalslotdialog.cpp shared/signalslotdialog.ui shared/signalslotdialog_p.h
        shared/spacer_widget.cpp shared/spacer_widget_p.h
        shared/stylesheeteditor.cpp shared/stylesheeteditor_p.h
        shared/textpropertyeditor.cpp shared/textpropertyeditor_p.h
        shared/widgetdatabase.cpp shared/widgetdatabase_p.h
        shared/widgetfactory.cpp shared/widgetfactory_p.h
        shared/zoomwidget.cpp shared/zoomwidget_p.h
        uilib/abstractformbuilder.cpp uilib/abstractformbuilder.h
        uilib/formbuilder.cpp uilib/formbuilder.h
        uilib/formbuilderextra.cpp uilib/formbuilderextra_p.h
        uilib/properties.cpp uilib/properties_p.h
        uilib/resourcebuilder.cpp uilib/resourcebuilder_p.h
        uilib/textbuilder.cpp uilib/textbuilder_p.h
        uilib/ui4.cpp uilib/ui4_p.h
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
)

# Resources:
set_source_files_properties("../../../shared/deviceskin/skins/ClamshellPhone.skin"
    PROPERTIES QT_RESOURCE_ALIAS "ClamshellPhone.skin"
)
set(ClamshellPhone_resource_files
    "ClamshellPhone.skin"
)

qt_internal_add_resource(Designer "ClamshellPhone"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${ClamshellPhone_resource_files}
)
set_source_files_properties("../../../shared/deviceskin/skins/SmartPhone2.skin"
    PROPERTIES QT_RESOURCE_ALIAS "SmartPhone2.skin"
)
set(SmartPhone2_resource_files
    "SmartPhone2.skin"
)

qt_internal_add_resource(Designer "SmartPhone2"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${SmartPhone2_resource_files}
)
set_source_files_properties("../../../shared/deviceskin/skins/SmartPhone.skin"
    PROPERTIES QT_RESOURCE_ALIAS "SmartPhone.skin"
)
set(SmartPhone_resource_files
    "SmartPhone.skin"
)

qt_internal_add_resource(Designer "SmartPhone"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${SmartPhone_resource_files}
)
set_source_files_properties("../../../shared/deviceskin/skins/SmartPhoneWithButtons.skin"
    PROPERTIES QT_RESOURCE_ALIAS "SmartPhoneWithButtons.skin"
)
set(SmartPhoneWithButtons_resource_files
    "SmartPhoneWithButtons.skin"
)

qt_internal_add_resource(Designer "SmartPhoneWithButtons"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${SmartPhoneWithButtons_resource_files}
)
set_source_files_properties("../../../shared/deviceskin/skins/TouchscreenPhone.skin"
    PROPERTIES QT_RESOURCE_ALIAS "TouchscreenPhone.skin"
)
set(TouchscreenPhone_resource_files
    "TouchscreenPhone.skin"
)

qt_internal_add_resource(Designer "TouchscreenPhone"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${TouchscreenPhone_resource_files}
)
set_source_files_properties("../../../shared/deviceskin/skins/PortableMedia.skin"
    PROPERTIES QT_RESOURCE_ALIAS "PortableMedia.skin"
)
set(PortableMedia_resource_files
    "PortableMedia.skin"
)

qt_internal_add_resource(Designer "PortableMedia"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${PortableMedia_resource_files}
)
set_source_files_properties("../../../shared/deviceskin/skins/S60-QVGA-Candybar.skin"
    PROPERTIES QT_RESOURCE_ALIAS "S60-QVGA-Candybar.skin"
)
set(S60-QVGA-Candybar_resource_files
    "S60-QVGA-Candybar.skin"
)

qt_internal_add_resource(Designer "S60-QVGA-Candybar"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${S60-QVGA-Candybar_resource_files}
)
set_source_files_properties("../../../shared/deviceskin/skins/S60-nHD-Touchscreen.skin"
    PROPERTIES QT_RESOURCE_ALIAS "S60-nHD-Touchscreen.skin"
)
set(S60-nHD-Touchscreen_resource_files
    "S60-nHD-Touchscreen.skin"
)

qt_internal_add_resource(Designer "S60-nHD-Touchscreen"
    PREFIX
        "/skins"
    BASE
        "../../../shared/deviceskin/skins"
    FILES
        ${S60-nHD-Touchscreen_resource_files}
)
set_source_files_properties("../../../shared/findwidget/images/mac/closetab.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/mac/closetab.png"
)
set_source_files_properties("../../../shared/findwidget/images/mac/next.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/mac/next.png"
)
set_source_files_properties("../../../shared/findwidget/images/mac/previous.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/mac/previous.png"
)
set_source_files_properties("../../../shared/findwidget/images/mac/searchfind.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/mac/searchfind.png"
)
set_source_files_properties("../../../shared/findwidget/images/win/closetab.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/win/closetab.png"
)
set_source_files_properties("../../../shared/findwidget/images/win/next.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/win/next.png"
)
set_source_files_properties("../../../shared/findwidget/images/win/previous.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/win/previous.png"
)
set_source_files_properties("../../../shared/findwidget/images/win/searchfind.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/win/searchfind.png"
)
set_source_files_properties("../../../shared/findwidget/images/wrap.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/wrap.png"
)
set(findwidget_resource_files
    "images/mac/closetab.png"
    "images/mac/next.png"
    "images/mac/previous.png"
    "images/mac/searchfind.png"
    "images/win/closetab.png"
    "images/win/next.png"
    "images/win/previous.png"
    "images/win/searchfind.png"
    "images/wrap.png"
)

qt_internal_add_resource(Designer "findwidget"
    PREFIX
        "/qt-project.org/shared"
    BASE
        "../../../shared/findwidget"
    FILES
        ${findwidget_resource_files}
)
set_source_files_properties("../../../shared/qtgradienteditor/images/down.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/down.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/edit.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/edit.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/editdelete.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/editdelete.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/minus.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/minus.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/plus.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/plus.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/spreadpad.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/spreadpad.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/spreadreflect.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/spreadreflect.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/spreadrepeat.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/spreadrepeat.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/typeconical.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/typeconical.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/typelinear.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/typelinear.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/typeradial.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/typeradial.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/up.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/up.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/zoomin.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/zoomin.png"
)
set_source_files_properties("../../../shared/qtgradienteditor/images/zoomout.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/zoomout.png"
)
set(qtgradienteditor_resource_files
    "images/down.png"
    "images/edit.png"
    "images/editdelete.png"
    "images/minus.png"
    "images/plus.png"
    "images/spreadpad.png"
    "images/spreadreflect.png"
    "images/spreadrepeat.png"
    "images/typeconical.png"
    "images/typelinear.png"
    "images/typeradial.png"
    "images/up.png"
    "images/zoomin.png"
    "images/zoomout.png"
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
    "defaultgradients.xml"
    "templates/forms/240x320/Dialog_with_Buttons_Bottom.ui"
    "templates/forms/240x320/Dialog_with_Buttons_Right.ui"
    "templates/forms/320x240/Dialog_with_Buttons_Bottom.ui"
    "templates/forms/320x240/Dialog_with_Buttons_Right.ui"
    "templates/forms/480x640/Dialog_with_Buttons_Bottom.ui"
    "templates/forms/480x640/Dialog_with_Buttons_Right.ui"
    "templates/forms/640x480/Dialog_with_Buttons_Bottom.ui"
    "templates/forms/640x480/Dialog_with_Buttons_Right.ui"
    "templates/forms/Dialog_with_Buttons_Bottom.ui"
    "templates/forms/Dialog_with_Buttons_Right.ui"
    "templates/forms/Dialog_without_Buttons.ui"
    "templates/forms/Main_Window.ui"
    "templates/forms/Widget.ui"
)

qt_internal_add_resource(Designer "shared"
    PREFIX
        "/qt-project.org/designer"
    BASE
        "shared"
    FILES
        ${shared_resource_files}
)


#### Keys ignored in scope 1:.:.:lib.pro:<TRUE>:
# MODULE = "designer"

## Scopes:
#####################################################################

qt_internal_extend_target(Designer CONDITION TARGET Qt::OpenGLWidgets
    PUBLIC_LIBRARIES
        Qt::OpenGLWidgets
)

qt_internal_extend_target(Designer CONDITION static
    DEFINES
        QT_DESIGNER_STATIC
)

qt_internal_extend_target(Designer CONDITION NOT QT_BUILD_SHARED_LIBS
    SOURCES
        ../../../shared/qtpropertybrowser/qtbuttonpropertybrowser.cpp ../../../shared/qtpropertybrowser/qtbuttonpropertybrowser.h
        ../../../shared/qtpropertybrowser/qteditorfactory.cpp ../../../shared/qtpropertybrowser/qteditorfactory.h
        ../../../shared/qtpropertybrowser/qtgroupboxpropertybrowser.cpp ../../../shared/qtpropertybrowser/qtgroupboxpropertybrowser.h
        ../../../shared/qtpropertybrowser/qtpropertybrowser.cpp ../../../shared/qtpropertybrowser/qtpropertybrowser.h
        ../../../shared/qtpropertybrowser/qtpropertybrowserutils.cpp ../../../shared/qtpropertybrowser/qtpropertybrowserutils_p.h
        ../../../shared/qtpropertybrowser/qtpropertymanager.cpp ../../../shared/qtpropertybrowser/qtpropertymanager.h
        ../../../shared/qtpropertybrowser/qttreepropertybrowser.cpp ../../../shared/qtpropertybrowser/qttreepropertybrowser.h
        ../../../shared/qtpropertybrowser/qtvariantproperty.cpp ../../../shared/qtpropertybrowser/qtvariantproperty.h
    INCLUDE_DIRECTORIES
        ../../../shared/qtpropertybrowser
)

if(NOT QT_BUILD_SHARED_LIBS)
    # Resources:
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-arrow.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-arrow.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-busy.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-busy.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-closedhand.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-closedhand.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-cross.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-cross.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-forbidden.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-forbidden.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-hand.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-hand.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-hsplit.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-hsplit.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-ibeam.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-ibeam.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-openhand.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-openhand.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-sizeall.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-sizeall.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-sizeb.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-sizeb.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-sizef.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-sizef.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-sizeh.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-sizeh.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-sizev.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-sizev.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-uparrow.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-uparrow.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-vsplit.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-vsplit.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-wait.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-wait.png"
    )
    set_source_files_properties("../../../shared/qtpropertybrowser/images/cursor-whatsthis.png"
        PROPERTIES QT_RESOURCE_ALIAS "images/cursor-whatsthis.png"
    )
    set(qtpropertybrowser_resource_files
        "images/cursor-arrow.png"
        "images/cursor-busy.png"
        "images/cursor-closedhand.png"
        "images/cursor-cross.png"
        "images/cursor-forbidden.png"
        "images/cursor-hand.png"
        "images/cursor-hsplit.png"
        "images/cursor-ibeam.png"
        "images/cursor-openhand.png"
        "images/cursor-sizeall.png"
        "images/cursor-sizeb.png"
        "images/cursor-sizef.png"
        "images/cursor-sizeh.png"
        "images/cursor-sizev.png"
        "images/cursor-uparrow.png"
        "images/cursor-vsplit.png"
        "images/cursor-wait.png"
        "images/cursor-whatsthis.png"
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
