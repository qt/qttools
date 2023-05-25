#! [1]
QT      += widgets designer
#! [1]

#! [0]
TEMPLATE = lib
CONFIG  += plugin
#! [0]

TARGET = $$qtLibraryTarget($$TARGET)

#! [3]
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
#! [3]

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
