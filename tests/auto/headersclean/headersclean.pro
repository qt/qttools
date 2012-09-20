TARGET = tst_headersclean
CONFIG += testcase
SOURCES  += tst_headersclean.cpp
QT = core testlib
!isEmpty(QT.help.name): QT += help
!isEmpty(QT.designer.name): QT += designer
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
