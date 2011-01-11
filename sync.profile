%modules = ( # path to module name map
    "QtHelp" => "$basedir/src/assistant/lib",
    "QtDesigner" => "$basedir/src/designer/src/lib",
    "QtUiTools" => "$basedir/src/designer/src/uitools",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
%mastercontent = (
    "sql" => "#include <QtSql/QtSql>\n",
    "xml" => "#include <QtXml/QtXml>\n",
    "network" => "#include <QtNetwork/QtNetwork>\n",
    "script" => "#include <QtScript/QtScript>\n",
    "qt3support" => "#include <Qt3Support/Qt3Support>\n",
    "declarative" => "#include <QtDeclarative/QtDeclarative>\n",
    "help" => "#include <QtHelp/QtHelp>\n",
    "designer" => "#include <QtDesigner/QtDesigner>\n",
    "uitools" => "#include <QtUiTools/QtUiTools>\n",
);
%modulepris = (
    "QtHelp" => "$basedir/modules/qt_help.pri",
    "QtDesigner" => "$basedir/modules/qt_designer.pri",
    "QtUiTools" => "$basedir/modules/qt_uitools.pri",
);
