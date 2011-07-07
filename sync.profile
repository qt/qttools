%modules = ( # path to module name map
    "QtCLucene" => "$basedir/src/assistant/lib/fulltextsearch",
    "QtHelp" => "$basedir/src/assistant/lib",
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
);
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
    "qtbase" => "refs/heads/master",
    "qtscript" => "refs/heads/master",
    "qtwebkit" => "refs/heads/qt-modularization-base",
    "qtscript" => "refs/heads/master",
    "qtsvg" => "refs/heads/master",
    "qtxmlpatterns" => "refs/heads/master",
    "qtdeclarative" => "refs/heads/master",
    "qtphonon" => "refs/heads/master",
    "qtactiveqt" => "refs/heads/master",
);
