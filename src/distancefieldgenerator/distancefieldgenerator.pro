QT += gui widgets gui-private core-private quick-private

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    distancefieldmodel.cpp \
    distancefieldmodelworker.cpp

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_FOREACH

FORMS += \
    mainwindow.ui

HEADERS += \
    mainwindow.h \
    distancefieldmodel.h \
    distancefieldmodelworker.h

