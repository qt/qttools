%modules = ( # path to module name map
    "QtTools" => "$basedir/src/global",
    "QtHelp" => "$basedir/src/assistant/help",
    "QtUiTools" => "$basedir/src/uitools",
    "QtUiPlugin" => "$basedir/src/uiplugin",
    "QtDesigner" => "$basedir/src/designer/src/lib",
    "QtDesignerComponents" => "$basedir/src/designer/src/components/lib",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
%deprecatedheaders = (
    "QtDesigner" => {
        "customwidget.h" => "QtUiPlugin/customwidget.h",
        "QDesignerCustomWidgetInterface" => "QtUiPlugin/QDesignerCustomWidgetInterface",
        "QDesignerCustomWidgetCollectionInterface" => "QtUiPlugin/QDesignerCustomWidgetCollectionInterface",
        "qdesignerexportwidget.h" => "QtUiPlugin/qdesignerexportwidget.h",
        "QDesignerExportWidget" => "QtUiPlugin/QDesignerExportWidget"
    }
);
