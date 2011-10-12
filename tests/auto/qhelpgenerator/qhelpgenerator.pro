TARGET = tst_qhelpgenerator
CONFIG += testcase

SOURCES += tst_qhelpgenerator.cpp
CONFIG  += help
QT      += sql testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_USE_USING_NAMESPACE
