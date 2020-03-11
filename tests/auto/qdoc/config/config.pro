CONFIG += testcase
QT = core testlib
TARGET = tst_config
INCLUDEPATH += $$PWD/../../../../src/qdoc

HEADERS += \
    $$PWD/../../../../src/qdoc/config.h \
    $$PWD/../../../../src/qdoc/location.h \
    $$PWD/../../../../src/qdoc/qdoccommandlineparser.h \
    $$PWD/../../../../src/qdoc/loggingcategory.h

SOURCES += \
    tst_config.cpp \
    $$PWD/../../../../src/qdoc/config.cpp \
    $$PWD/../../../../src/qdoc/location.cpp \
    $$PWD/../../../../src/qdoc/qdoccommandlineparser.cpp
