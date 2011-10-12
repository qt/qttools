TARGET = tst_qhelpindexmodel
CONFIG += testcase

SOURCES += tst_qhelpindexmodel.cpp
CONFIG  += help
QT      += sql testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_USE_USING_NAMESPACE
