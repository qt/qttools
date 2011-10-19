CONFIG += testcase

TEMPLATE = app
!embedded:CONFIG += uitools
TARGET = ../tst_uiloader
DEFINES += SRCDIR=\\\"$$PWD\\\"

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_uiloader
} else {
    TARGET = ../../release/tst_uiloader
  }
}

QT += widgets network testlib

# Input
HEADERS += uiloader.h
SOURCES += tst_uiloader.cpp uiloader.cpp
