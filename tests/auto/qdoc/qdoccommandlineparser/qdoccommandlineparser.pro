CONFIG += testcase
QT = core testlib
TARGET = tst_qdoccommandlineparser
INCLUDEPATH += $$PWD/../../../../src/qdoc

HEADERS += \
    $$PWD/../../../../src/qdoc/qdoccommandlineparser.h

SOURCES += \
    $$PWD/../../../../src/qdoc/qdoccommandlineparser.cpp \
    tst_qdoccommandlineparser.cpp

RESOURCES += \
    tst_qdoccommandlineparser.qrc
