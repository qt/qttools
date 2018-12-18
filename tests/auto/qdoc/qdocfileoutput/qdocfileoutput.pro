CONFIG += testcase
QT = core testlib

TARGET = tst_qdocfileoutput

SOURCES += \
    tst_qdocfileoutput.cpp

QMAKE_DOCS = $$PWD/test.qdocconf
