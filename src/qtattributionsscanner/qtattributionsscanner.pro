CONFIG += console

SOURCES += \
    jsongenerator.cpp \
    main.cpp \
    qdocgenerator.cpp \
    scanner.cpp

HEADERS += \
    jsongenerator.h \
    logging.h \
    package.h \
    qdocgenerator.h \
    scanner.h

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

option(host_build)
load(qt_app)
