QT += xml \
    network \
    help
TEMPLATE = app
DESTDIR = $$QT.designer.bins
TARGET = qcollectiongenerator
CONFIG += qt \
    warn_on \
    help \
    console
CONFIG -= app_bundle
target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
SOURCES += ../shared/helpgenerator.cpp \
    main.cpp \
    ../shared/collectionconfiguration.cpp
HEADERS += ../shared/helpgenerator.h \
    ../shared/collectionconfiguration.h
