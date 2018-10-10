CONFIG += testcase
QT = core testlib
TARGET = tst_qdocglobals
INCLUDEPATH += $$PWD/../../../../src/qdoc

HEADERS += $$PWD/../../../../src/qdoc/qdocglobals.h

SOURCES += $$PWD/../../../../src/qdoc/qdocglobals.cpp \
           tst_qdocglobals.cpp
