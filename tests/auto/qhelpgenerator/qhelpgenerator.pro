TARGET = tst_qhelpgenerator
CONFIG += testcase

HEADERS += ../../../src/assistant/shared/helpgenerator.h
SOURCES += tst_qhelpgenerator.cpp \
           ../../../src/assistant/shared/helpgenerator.cpp
QT      += help-private sql testlib

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_USE_USING_NAMESPACE
