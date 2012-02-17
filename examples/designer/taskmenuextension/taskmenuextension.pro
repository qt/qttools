#! [0]
TEMPLATE = lib
#! [0]
TARGET   = $$qtLibraryTarget($$TARGET)
#! [1]
CONFIG  += designer plugin
QT      += widgets
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
OTHER_FILES += tictactoe.json
#! [2]

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/taskmenuextension
INSTALLS += target sources
