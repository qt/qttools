TARGET = tst_tests
CONFIG += testcase

QT += xml testlib

HEADERS += \
    tst_linguist.h \
    ../shared/translator.h

SOURCES +=  \
    tst_linguist.cpp \
    tst_lupdate.cpp \
    tst_simtexth.cpp \
    ../shared/simtexth.cpp \
    ../shared/translator.cpp \
    ../shared/translatormessage.cpp \
    ../shared/xliff.cpp
