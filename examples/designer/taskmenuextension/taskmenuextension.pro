#! [0]
TEMPLATE = lib
#! [0]
TARGET   = $$qtLibraryTarget($$TARGET)
#! [1]
CONFIG  += designer plugin
#! [1]
DESTDIR = $$QT.designer.plugins/designer

#! [2]
HEADERS += tictactoe.h \
           tictactoedialog.h \
           tictactoeplugin.h \
           tictactoetaskmenu.h
SOURCES += tictactoe.cpp \
           tictactoedialog.cpp \
           tictactoeplugin.cpp \
           tictactoetaskmenu.cpp
#! [2]

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/taskmenuextension
INSTALLS += target sources

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example
symbian: warning(This example does not work on Symbian platform)
maemo5: warning(This example does not work on Maemo platform)
simulator: warning(This example does not work on Simulator platform)
