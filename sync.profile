%modules = ( # path to module name map
    "QtCLucene" => "$basedir/src/assistant/clucene",
    "QtHelp" => "$basedir/src/assistant/help",
    "QtDesigner" => "$basedir/src/designer/src/lib",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
    "qtcluceneversion.h" => "QtCLuceneVersion",
    "qthelpversion.h" => "QtHelpVersion",
    "qtdesignerversion.h" => "QtDesigner",
);
%mastercontent = (
    "sql" => "#include <QtSql/QtSql>\n",
    "xml" => "#include <QtXml/QtXml>\n",
    "network" => "#include <QtNetwork/QtNetwork>\n",
    "script" => "#include <QtScript/QtScript>\n",
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
);
%dependencies = (
    	"qtbase" => "refs/heads/master",
        "qtsvg" => "refs/heads/master",
        "qtxmlpatterns" => "refs/heads/master",
);
