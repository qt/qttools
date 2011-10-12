TARGET = tst_qhelpprojectdata
CONFIG += testcase

SOURCES += tst_qhelpprojectdata.cpp
CONFIG  += help
QT      += testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_USE_USING_NAMESPACE

