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
# Modules and programs, and their dependencies.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - "LATEST_REVISION", to always test against the latest revision.
#   - "LATEST_RELEASE", to always test against the latest public release.
#   - "THIS_REPOSITORY", to indicate that the module is in this repository.
%dependencies = (
    "QtHelp" => {
        "QtCore" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtGui" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtSql" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtXml" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
    },
    "linguist" => {
        "QtGui" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtDesigner" => "THIS_REPOSITORY",
        "QtUiTools" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtXml" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtCore" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
    },
    "designer" => {
        "QtNetwork" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtGui" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtXml" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtDesigner" => "THIS_REPOSITORY",
        "QtScript" => "4d15ca64fc7ca81bdadba9fbeb84d4e98a6c0edc",
        "QtCore" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
    },
    "assistant" => {
        "QtWebKit" => "LATEST_REVISION",
        "QtHelp" => "THIS_REPOSITORY",
        "QtNetwork" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtGui" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtSql" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtXml" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtCore" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
    },
    "QtDesigner" => {
        "QtCore" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtGui" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
        "QtScript" => "4d15ca64fc7ca81bdadba9fbeb84d4e98a6c0edc",
        "QtXml" => "0c637cb07ba3c9b353e7e483a209537485cc4e2a",
    },
);
