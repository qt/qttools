CONFIG += testcase
QT = core testlib
TARGET = tst_qdocglobals

include($$PWD/../../../../src/qdoc/qdoc.pri)

SOURCES += tst_qdocglobals.cpp
