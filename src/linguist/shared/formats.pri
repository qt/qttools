INCLUDEPATH *= $$PWD

SOURCES += \
    $$PWD/numerus.cpp \
    $$PWD/translator.cpp \
    $$PWD/translatormessage.cpp \
    $$PWD/xmlparser.cpp

HEADERS += \
    $$PWD/translator.h \
    $$PWD/translatormessage.h \
    $$PWD/xmlparser.h

# "real" formats readers and writers
SOURCES += \
    $$PWD/qm.cpp \
    $$PWD/qph.cpp \
    $$PWD/po.cpp \
    $$PWD/ts.cpp \
    $$PWD/xliff.cpp
