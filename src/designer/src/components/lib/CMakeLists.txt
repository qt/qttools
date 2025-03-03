# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## DesignerComponentsPrivate Module:
#####################################################################

qt_internal_add_module(DesignerComponentsPrivate
    INTERNAL_MODULE
    SOURCES
        lib_pch.h
        ../buddyeditor/buddyeditor.cpp ../buddyeditor/buddyeditor.h
        ../buddyeditor/buddyeditor_global.h
        ../buddyeditor/buddyeditor_plugin.cpp ../buddyeditor/buddyeditor_plugin.h
        ../buddyeditor/buddyeditor_tool.cpp ../buddyeditor/buddyeditor_tool.h
        ../formeditor/default_actionprovider.cpp ../formeditor/default_actionprovider.h
        ../formeditor/default_container.cpp ../formeditor/default_container.h
        ../formeditor/default_layoutdecoration.cpp ../formeditor/default_layoutdecoration.h
        ../formeditor/deviceprofiledialog.cpp ../formeditor/deviceprofiledialog.h
        ../formeditor/dpi_chooser.cpp ../formeditor/dpi_chooser.h
        ../formeditor/embeddedoptionspage.cpp ../formeditor/embeddedoptionspage.h
        ../formeditor/formeditor.cpp ../formeditor/formeditor.h
        ../formeditor/formeditor_global.h
        ../formeditor/formeditor_optionspage.cpp ../formeditor/formeditor_optionspage.h
        ../formeditor/formwindow.cpp ../formeditor/formwindow.h
        ../formeditor/formwindow_dnditem.cpp ../formeditor/formwindow_dnditem.h
        ../formeditor/formwindow_widgetstack.cpp ../formeditor/formwindow_widgetstack.h
        ../formeditor/formwindowcursor.cpp ../formeditor/formwindowcursor.h
        ../formeditor/formwindowmanager.cpp ../formeditor/formwindowmanager.h
        ../formeditor/formwindowsettings.cpp ../formeditor/formwindowsettings.h
        ../formeditor/itemview_propertysheet.cpp ../formeditor/itemview_propertysheet.h
        ../formeditor/layout_propertysheet.cpp ../formeditor/layout_propertysheet.h
        ../formeditor/line_propertysheet.cpp ../formeditor/line_propertysheet.h
        ../formeditor/previewactiongroup.cpp ../formeditor/previewactiongroup.h
        ../formeditor/qdesigner_resource.cpp ../formeditor/qdesigner_resource.h
        ../formeditor/qlayoutwidget_propertysheet.cpp ../formeditor/qlayoutwidget_propertysheet.h
        ../formeditor/qmainwindow_container.cpp ../formeditor/qmainwindow_container.h
        ../formeditor/qmdiarea_container.cpp ../formeditor/qmdiarea_container.h
        ../formeditor/qwizard_container.cpp ../formeditor/qwizard_container.h
        ../formeditor/spacer_propertysheet.cpp ../formeditor/spacer_propertysheet.h
        ../formeditor/templateoptionspage.cpp ../formeditor/templateoptionspage.h
        ../formeditor/tool_widgeteditor.cpp ../formeditor/tool_widgeteditor.h
        ../formeditor/widgetselection.cpp ../formeditor/widgetselection.h
        ../objectinspector/objectinspector.cpp ../objectinspector/objectinspector.h
        ../objectinspector/objectinspector_global.h
        ../objectinspector/objectinspectormodel.cpp ../objectinspector/objectinspectormodel_p.h
        ../propertyeditor/brushpropertymanager.cpp ../propertyeditor/brushpropertymanager.h
        ../propertyeditor/designerpropertymanager.cpp ../propertyeditor/designerpropertymanager.h
        ../propertyeditor/fontpropertymanager.cpp ../propertyeditor/fontpropertymanager.h
        ../propertyeditor/newdynamicpropertydialog.cpp ../propertyeditor/newdynamicpropertydialog.h
        ../propertyeditor/paletteeditor.cpp ../propertyeditor/paletteeditor.h
        ../propertyeditor/paletteeditorbutton.cpp ../propertyeditor/paletteeditorbutton.h
        ../propertyeditor/pixmapeditor.cpp ../propertyeditor/pixmapeditor.h
        ../propertyeditor/previewframe.cpp ../propertyeditor/previewframe.h
        ../propertyeditor/previewwidget.cpp ../propertyeditor/previewwidget.h
        ../propertyeditor/propertyeditor.cpp ../propertyeditor/propertyeditor.h
        ../propertyeditor/propertyeditor_global.h
        ../propertyeditor/qlonglongvalidator.cpp ../propertyeditor/qlonglongvalidator.h
        ../propertyeditor/resetdecorator.cpp ../propertyeditor/resetdecorator.h
        ../propertyeditor/texteditor.cpp ../propertyeditor/texteditor.h
        ../propertyeditor/stringlisteditor.cpp ../propertyeditor/stringlisteditor.h
        ../propertyeditor/stringlisteditorbutton.cpp ../propertyeditor/stringlisteditorbutton.h
        ../signalsloteditor/connectdialog.cpp  ../signalsloteditor/connectdialog_p.h
        ../signalsloteditor/signalslot_utils.cpp ../signalsloteditor/signalslot_utils_p.h
        ../signalsloteditor/signalsloteditor.cpp ../signalsloteditor/signalsloteditor.h ../signalsloteditor/signalsloteditor_p.h
        ../signalsloteditor/signalsloteditor_global.h
        ../signalsloteditor/signalsloteditor_plugin.cpp ../signalsloteditor/signalsloteditor_plugin.h
        ../signalsloteditor/signalsloteditor_tool.cpp ../signalsloteditor/signalsloteditor_tool.h
        ../signalsloteditor/signalsloteditorwindow.cpp ../signalsloteditor/signalsloteditorwindow.h
        ../tabordereditor/tabordereditor.cpp ../tabordereditor/tabordereditor.h
        ../tabordereditor/tabordereditor_global.h
        ../tabordereditor/tabordereditor_plugin.cpp ../tabordereditor/tabordereditor_plugin.h
        ../tabordereditor/tabordereditor_tool.cpp ../tabordereditor/tabordereditor_tool.h
        ../taskmenu/button_taskmenu.cpp ../taskmenu/button_taskmenu.h
        ../taskmenu/combobox_taskmenu.cpp ../taskmenu/combobox_taskmenu.h
        ../taskmenu/containerwidget_taskmenu.cpp ../taskmenu/containerwidget_taskmenu.h
        ../taskmenu/groupbox_taskmenu.cpp ../taskmenu/groupbox_taskmenu.h
        ../taskmenu/inplace_editor.cpp ../taskmenu/inplace_editor.h
        ../taskmenu/inplace_widget_helper.cpp ../taskmenu/inplace_widget_helper.h
        ../taskmenu/itemlisteditor.cpp ../taskmenu/itemlisteditor.h
        ../taskmenu/label_taskmenu.cpp ../taskmenu/label_taskmenu.h
        ../taskmenu/layouttaskmenu.cpp ../taskmenu/layouttaskmenu.h
        ../taskmenu/lineedit_taskmenu.cpp ../taskmenu/lineedit_taskmenu.h
        ../taskmenu/listwidget_taskmenu.cpp ../taskmenu/listwidget_taskmenu.h
        ../taskmenu/listwidgeteditor.cpp ../taskmenu/listwidgeteditor.h
        ../taskmenu/menutaskmenu.cpp ../taskmenu/menutaskmenu.h
        ../taskmenu/tablewidget_taskmenu.cpp ../taskmenu/tablewidget_taskmenu.h
        ../taskmenu/tablewidgeteditor.cpp ../taskmenu/tablewidgeteditor.h
        ../taskmenu/taskmenu_component.cpp ../taskmenu/taskmenu_component.h
        ../taskmenu/textedit_taskmenu.cpp ../taskmenu/textedit_taskmenu.h
        ../taskmenu/toolbar_taskmenu.cpp ../taskmenu/toolbar_taskmenu.h
        ../taskmenu/treewidget_taskmenu.cpp ../taskmenu/treewidget_taskmenu.h
        ../taskmenu/treewidgeteditor.cpp ../taskmenu/treewidgeteditor.h
        ../widgetbox/widgetbox.cpp ../widgetbox/widgetbox.h
        ../widgetbox/widgetbox_dnditem.cpp ../widgetbox/widgetbox_dnditem.h
        ../widgetbox/widgetbox_global.h
        ../widgetbox/widgetboxcategorylistview.cpp ../widgetbox/widgetboxcategorylistview.h
        ../widgetbox/widgetboxtreewidget.cpp ../widgetbox/widgetboxtreewidget.h
        qdesigner_components.cpp
    NO_UNITY_BUILD_SOURCES
        ../tabordereditor/tabordereditor.cpp # redefinition of 'QMetaTypeId<QList<QWidget *>>' (from qdesigner_resource.cpp)
        ../formeditor/formwindow.cpp         # explicit specialization of 'QMetaTypeId<QWidget *>' after instantiation
    DEFINES
        QDESIGNER_COMPONENTS_LIBRARY
        QT_STATICPLUGIN
    INCLUDE_DIRECTORIES
        .
        ..
        ../../../../shared/tools/shared/qtpropertybrowser
        ../../lib/components
        ../../lib/extension
        ../../lib/sdk
        ../../lib/shared
        ../buddyeditor
        ../formeditor
        ../objectinspector
        ../propertyeditor ../propertyeditor
        ../signalsloteditor
        ../tabordereditor
        ../taskmenu
        ../widgetbox
    LIBRARIES
        Qt::Xml
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::DesignerPrivate
        Qt::GuiPrivate
        Qt::WidgetsPrivate
        Qt::Xml
    ENABLE_AUTOGEN_TOOLS
        uic
    PRECOMPILED_HEADER
        "lib_pch.h"
    NO_GENERATE_CPP_EXPORTS
)

set(ui_sources
    ../formeditor/deviceprofiledialog.ui
    ../formeditor/formwindowsettings.ui
    ../formeditor/templateoptionspage.ui
    ../propertyeditor/newdynamicpropertydialog.ui
    ../propertyeditor/paletteeditor.ui
    ../propertyeditor/previewwidget.ui
    ../propertyeditor/stringlisteditor.ui
    ../signalsloteditor/connectdialog.ui
    ../taskmenu/itemlisteditor.ui
    ../taskmenu/tablewidgeteditor.ui
    ../taskmenu/treewidgeteditor.ui
)

# Work around QTBUG-95305
if(CMAKE_GENERATOR STREQUAL "Ninja Multi-Config" AND CMAKE_CROSS_CONFIGS)
    qt6_wrap_ui(ui_sources_processed ${ui_sources})
else()
    set(ui_sources_processed ${ui_sources})
endif()
target_sources(DesignerComponentsPrivate PRIVATE ${ui_sources_processed})

# Resources:
set(propertyeditor_resource_files
    "../propertyeditor/fontmapping.xml"
)

qt_internal_add_resource(DesignerComponentsPrivate "propertyeditor"
    PREFIX
        "/qt-project.org/propertyeditor"
    BASE
        "../propertyeditor"
    FILES
        ${propertyeditor_resource_files}
)
set(formeditor_resource_files
    "../formeditor/images/color.png"
    "../formeditor/images/configure.png"
    "../formeditor/images/downplus.png"
    "../formeditor/images/dropdownbutton.png"
    "../formeditor/images/edit.png"
    "../formeditor/images/editdelete-16.png"
    "../formeditor/images/emptyicon.png"
    "../formeditor/images/filenew-16.png"
    "../formeditor/images/fileopen-16.png"
    "../formeditor/images/leveldown.png"
    "../formeditor/images/levelup.png"
    "../formeditor/images/mac/adjustsize.png"
    "../formeditor/images/mac/back.png"
    "../formeditor/images/mac/buddytool.png"
    "../formeditor/images/mac/down.png"
    "../formeditor/images/mac/editbreaklayout.png"
    "../formeditor/images/mac/editcopy.png"
    "../formeditor/images/mac/editcut.png"
    "../formeditor/images/mac/editdelete.png"
    "../formeditor/images/mac/editform.png"
    "../formeditor/images/mac/editgrid.png"
    "../formeditor/images/mac/edithlayout.png"
    "../formeditor/images/mac/edithlayoutsplit.png"
    "../formeditor/images/mac/editlower.png"
    "../formeditor/images/mac/editpaste.png"
    "../formeditor/images/mac/editraise.png"
    "../formeditor/images/mac/editvlayout.png"
    "../formeditor/images/mac/editvlayoutsplit.png"
    "../formeditor/images/mac/filenew.png"
    "../formeditor/images/mac/fileopen.png"
    "../formeditor/images/mac/filesave.png"
    "../formeditor/images/mac/forward.png"
    "../formeditor/images/mac/insertimage.png"
    "../formeditor/images/mac/minus.png"
    "../formeditor/images/mac/plus.png"
    "../formeditor/images/mac/redo.png"
    "../formeditor/images/mac/signalslottool.png"
    "../formeditor/images/mac/simplifyrichtext.png"
    "../formeditor/images/mac/tabordertool.png"
    "../formeditor/images/mac/textanchor.png"
    "../formeditor/images/mac/textbold.png"
    "../formeditor/images/mac/textcenter.png"
    "../formeditor/images/mac/textitalic.png"
    "../formeditor/images/mac/textjustify.png"
    "../formeditor/images/mac/textleft.png"
    "../formeditor/images/mac/textright.png"
    "../formeditor/images/mac/textsubscript.png"
    "../formeditor/images/mac/textsuperscript.png"
    "../formeditor/images/mac/textunder.png"
    "../formeditor/images/mac/undo.png"
    "../formeditor/images/mac/up.png"
    "../formeditor/images/mac/widgettool.png"
    "../formeditor/images/minus-16.png"
    "../formeditor/images/prefix-add.png"
    "../formeditor/images/qtlogo128x128.png"
    "../formeditor/images/qtlogo16x16.png"
    "../formeditor/images/qtlogo24x24.png"
    "../formeditor/images/qtlogo32x32.png"
    "../formeditor/images/qtlogo64x64.png"
    "../formeditor/images/reload.png"
    "../formeditor/images/resetproperty.png"
    "../formeditor/images/righttoleft.png"
    "../formeditor/images/sort.png"
    "../formeditor/images/submenu.png"
    "../formeditor/images/widgets/calendarwidget.png"
    "../formeditor/images/widgets/checkbox.png"
    "../formeditor/images/widgets/columnview.png"
    "../formeditor/images/widgets/combobox.png"
    "../formeditor/images/widgets/commandlinkbutton.png"
    "../formeditor/images/widgets/dateedit.png"
    "../formeditor/images/widgets/datetimeedit.png"
    "../formeditor/images/widgets/dial.png"
    "../formeditor/images/widgets/dialogbuttonbox.png"
    "../formeditor/images/widgets/dockwidget.png"
    "../formeditor/images/widgets/doublespinbox.png"
    "../formeditor/images/widgets/fontcombobox.png"
    "../formeditor/images/widgets/frame.png"
    "../formeditor/images/widgets/graphicsview.png"
    "../formeditor/images/widgets/groupbox.png"
    "../formeditor/images/widgets/hscrollbar.png"
    "../formeditor/images/widgets/hslider.png"
    "../formeditor/images/widgets/label.png"
    "../formeditor/images/widgets/lcdnumber.png"
    "../formeditor/images/widgets/line.png"
    "../formeditor/images/widgets/lineedit.png"
    "../formeditor/images/widgets/listbox.png"
    "../formeditor/images/widgets/listview.png"
    "../formeditor/images/widgets/mdiarea.png"
    "../formeditor/images/widgets/plaintextedit.png"
    "../formeditor/images/widgets/progress.png"
    "../formeditor/images/widgets/pushbutton.png"
    "../formeditor/images/widgets/radiobutton.png"
    "../formeditor/images/widgets/scrollarea.png"
    "../formeditor/images/widgets/spacer.png"
    "../formeditor/images/widgets/spinbox.png"
    "../formeditor/images/widgets/table.png"
    "../formeditor/images/widgets/tabwidget.png"
    "../formeditor/images/widgets/textedit.png"
    "../formeditor/images/widgets/timeedit.png"
    "../formeditor/images/widgets/toolbox.png"
    "../formeditor/images/widgets/toolbutton.png"
    "../formeditor/images/widgets/vline.png"
    "../formeditor/images/widgets/vscrollbar.png"
    "../formeditor/images/widgets/vslider.png"
    "../formeditor/images/widgets/vspacer.png"
    "../formeditor/images/widgets/widget.png"
    "../formeditor/images/widgets/widgetstack.png"
    "../formeditor/images/win/adjustsize.png"
    "../formeditor/images/win/back.png"
    "../formeditor/images/win/buddytool.png"
    "../formeditor/images/win/down.png"
    "../formeditor/images/win/editbreaklayout.png"
    "../formeditor/images/win/editcopy.png"
    "../formeditor/images/win/editcut.png"
    "../formeditor/images/win/editdelete.png"
    "../formeditor/images/win/editform.png"
    "../formeditor/images/win/editgrid.png"
    "../formeditor/images/win/edithlayout.png"
    "../formeditor/images/win/edithlayoutsplit.png"
    "../formeditor/images/win/editlower.png"
    "../formeditor/images/win/editpaste.png"
    "../formeditor/images/win/editraise.png"
    "../formeditor/images/win/editvlayout.png"
    "../formeditor/images/win/editvlayoutsplit.png"
    "../formeditor/images/win/filenew.png"
    "../formeditor/images/win/fileopen.png"
    "../formeditor/images/win/filesave.png"
    "../formeditor/images/win/forward.png"
    "../formeditor/images/win/insertimage.png"
    "../formeditor/images/win/minus.png"
    "../formeditor/images/win/plus.png"
    "../formeditor/images/win/redo.png"
    "../formeditor/images/win/signalslottool.png"
    "../formeditor/images/win/simplifyrichtext.png"
    "../formeditor/images/win/tabordertool.png"
    "../formeditor/images/win/textanchor.png"
    "../formeditor/images/win/textbold.png"
    "../formeditor/images/win/textcenter.png"
    "../formeditor/images/win/textitalic.png"
    "../formeditor/images/win/textjustify.png"
    "../formeditor/images/win/textleft.png"
    "../formeditor/images/win/textright.png"
    "../formeditor/images/win/textsubscript.png"
    "../formeditor/images/win/textsuperscript.png"
    "../formeditor/images/win/textunder.png"
    "../formeditor/images/win/undo.png"
    "../formeditor/images/win/up.png"
    "../formeditor/images/win/widgettool.png"
)

qt_internal_add_resource(DesignerComponentsPrivate "formeditor"
    PREFIX
        "/qt-project.org/formeditor"
    BASE
        "../formeditor"
    FILES
        ${formeditor_resource_files}
)
set(formeditor1_resource_files
    "../formeditor/defaultbrushes.xml"
)

qt_internal_add_resource(DesignerComponentsPrivate "formeditor1"
    PREFIX
        "/qt-project.org/brushes"
    BASE
        "../formeditor"
    FILES
        ${formeditor1_resource_files}
)
set(widgetbox_resource_files
    "../widgetbox/widgetbox.xml"
)

qt_internal_add_resource(DesignerComponentsPrivate "widgetbox"
    PREFIX
        "/qt-project.org/widgetbox"
    BASE
        "../widgetbox"
    FILES
        ${widgetbox_resource_files}
)

## Scopes:
#####################################################################

qt_internal_extend_target(DesignerComponentsPrivate CONDITION NOT QT_BUILD_SHARED_LIBS
    DEFINES
        QT_DESIGNER_STATIC
    INCLUDE_DIRECTORIES
        ../../../../shared/findwidget
        ../../../../shared/qtgradienteditor
        ../../../../shared/qtpropertybrowser
)

qt_internal_extend_target(DesignerComponentsPrivate CONDITION QT_BUILD_SHARED_LIBS
    SOURCES
        ../../../../shared/findwidget/abstractfindwidget.cpp ../../../../shared/findwidget/abstractfindwidget_p.h
        ../../../../shared/findwidget/itemviewfindwidget.cpp ../../../../shared/findwidget/itemviewfindwidget_p.h
        ../../../../shared/findwidget/texteditfindwidget.cpp ../../../../shared/findwidget/texteditfindwidget_p.h
        ../../../../shared/qtgradienteditor/qtcolorbutton.cpp ../../../../shared/qtgradienteditor/qtcolorbutton_p.h
        ../../../../shared/qtpropertybrowser/qtbuttonpropertybrowser.cpp ../../../../shared/qtpropertybrowser/qtbuttonpropertybrowser_p.h
        ../../../../shared/qtpropertybrowser/qteditorfactory.cpp ../../../../shared/qtpropertybrowser/qteditorfactory_p.h
        ../../../../shared/qtpropertybrowser/qtgroupboxpropertybrowser.cpp ../../../../shared/qtpropertybrowser/qtgroupboxpropertybrowser_p.h
        ../../../../shared/qtpropertybrowser/qtpropertybrowser.cpp ../../../../shared/qtpropertybrowser/qtpropertybrowser_p.h
        ../../../../shared/qtpropertybrowser/qtpropertybrowserutils.cpp ../../../../shared/qtpropertybrowser/qtpropertybrowserutils_p.h
        ../../../../shared/qtpropertybrowser/qtpropertymanager.cpp ../../../../shared/qtpropertybrowser/qtpropertymanager_p.h
        ../../../../shared/qtpropertybrowser/qttreepropertybrowser.cpp ../../../../shared/qtpropertybrowser/qttreepropertybrowser_p.h
        ../../../../shared/qtpropertybrowser/qtvariantproperty.cpp ../../../../shared/qtpropertybrowser/qtvariantproperty_p.h
    INCLUDE_DIRECTORIES
        ../../../../shared/findwidget
        ../../../../shared/qtgradienteditor
        ../../../../shared/qtpropertybrowser
)

if(QT_BUILD_SHARED_LIBS)
    # Resources:
    set(findwidget_resource_files
        "../../../../shared/findwidget/images/mac/closetab.png"
        "../../../../shared/findwidget/images/mac/next.png"
        "../../../../shared/findwidget/images/mac/previous.png"
        "../../../../shared/findwidget/images/mac/searchfind.png"
        "../../../../shared/findwidget/images/win/closetab.png"
        "../../../../shared/findwidget/images/win/next.png"
        "../../../../shared/findwidget/images/win/previous.png"
        "../../../../shared/findwidget/images/win/searchfind.png"
        "../../../../shared/findwidget/images/wrap.png"
    )

    qt_internal_add_resource(DesignerComponentsPrivate "findwidget"
        PREFIX
            "/qt-project.org/shared"
        BASE
            "../../../../shared/findwidget"
        FILES
            ${findwidget_resource_files}
    )
    set(qtpropertybrowser_resource_files
        "../../../../shared/qtpropertybrowser/images/cursor-arrow.png"
        "../../../../shared/qtpropertybrowser/images/cursor-busy.png"
        "../../../../shared/qtpropertybrowser/images/cursor-closedhand.png"
        "../../../../shared/qtpropertybrowser/images/cursor-cross.png"
        "../../../../shared/qtpropertybrowser/images/cursor-forbidden.png"
        "../../../../shared/qtpropertybrowser/images/cursor-hand.png"
        "../../../../shared/qtpropertybrowser/images/cursor-hsplit.png"
        "../../../../shared/qtpropertybrowser/images/cursor-ibeam.png"
        "../../../../shared/qtpropertybrowser/images/cursor-openhand.png"
        "../../../../shared/qtpropertybrowser/images/cursor-sizeall.png"
        "../../../../shared/qtpropertybrowser/images/cursor-sizeb.png"
        "../../../../shared/qtpropertybrowser/images/cursor-sizef.png"
        "../../../../shared/qtpropertybrowser/images/cursor-sizeh.png"
        "../../../../shared/qtpropertybrowser/images/cursor-sizev.png"
        "../../../../shared/qtpropertybrowser/images/cursor-uparrow.png"
        "../../../../shared/qtpropertybrowser/images/cursor-vsplit.png"
        "../../../../shared/qtpropertybrowser/images/cursor-wait.png"
        "../../../../shared/qtpropertybrowser/images/cursor-whatsthis.png"
    )

    qt_internal_add_resource(DesignerComponentsPrivate "qtpropertybrowser"
        PREFIX
            "/qt-project.org/qtpropertybrowser"
        BASE
            "../../../../shared/qtpropertybrowser"
        FILES
            ${qtpropertybrowser_resource_files}
    )
endif()
