%modules = ( # path to module name map
    "QtCLucene" => "$basedir/src/assistant/clucene",
    "QtHelp" => "$basedir/src/assistant/help",
    "QtUiTools" => "$basedir/src/designer/src/uitools",
    "QtDesigner" => "$basedir/src/designer/src/lib",
    "QtDesignerComponents" => "$basedir/src/designer/src/components/lib",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
);
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
    "qtbase" => "refs/heads/release",
    "qtxmlpatterns" => "refs/heads/release",
    "qtdeclarative" => "refs/heads/release",
    "qtjsbackend" => "refs/heads/release",
    "qtactiveqt" => "refs/heads/release",
    "qtwebkit" => "refs/heads/release",
);
