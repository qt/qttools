%modules = ( # path to module name map
    "QtCLucene" => "$basedir/src/assistant/lib/fulltextsearch",
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
    "clucene" => "#include <QtCLucene/QtCLucene>\n",
    "help" => "#include <QtHelp/QtHelp>\n",
    "designer" => "#include <QtDesigner/QtDesigner>\n",
    "uitools" => "#include <QtUiTools/QtUiTools>\n",
);
%modulepris = (
    "QtCLucene" => "$basedir/modules/qt_clucene.pri",
    "QtHelp" => "$basedir/modules/qt_help.pri",
    "QtDesigner" => "$basedir/modules/qt_designer.pri",
    "QtUiTools" => "$basedir/modules/qt_uitools.pri",
);
