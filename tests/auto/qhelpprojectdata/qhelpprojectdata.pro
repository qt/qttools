TARGET = tst_qhelpprojectdata
CONFIG += testcase

SOURCES += tst_qhelpprojectdata.cpp
QT      += help testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_USE_USING_NAMESPACE

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
