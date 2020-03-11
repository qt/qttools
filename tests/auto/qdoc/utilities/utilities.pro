CONFIG += testcase
QT = core testlib
TARGET = tst_utilities
INCLUDEPATH += $$PWD/../../../../src/qdoc

HEADERS += \
    $$PWD/../../../../src/qdoc/loggingcategory.h \
    $$PWD/../../../../src/qdoc/utilities.h

SOURCES += \
    tst_utilities.cpp \
    $$PWD/../../../../src/qdoc/utilities.cpp
