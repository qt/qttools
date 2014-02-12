option(host_build)
CONFIG += force_bootstrap

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII WINRT_LIBRARY

SOURCES += main.cpp runner.cpp
HEADERS += runner.h runnerengine.h

DEFINES += RTRUNNER_NO_APPX RTRUNNER_NO_XAP

win32-msvc2012|win32-msvc2013 {
    SOURCES += appxengine.cpp
    HEADERS += appxengine.h
    LIBS += -lruntimeobject
    DEFINES -= RTRUNNER_NO_APPX

    # XAP libraries only support 32-bit
    equals(QT_ARCH, i386) {
        include(../shared/corecon/corecon.pri)
        SOURCES += xapengine.cpp
        HEADERS += xapengine.h
        DEFINES -= RTRUNNER_NO_XAP

        # Use zip class from qtbase
        SOURCES += \
            qzip/qzip.cpp
        HEADERS += \
            qzip/qzipreader_p.h \
            qzip/qzipwriter_p.h
        INCLUDEPATH += qzip $$[QT_INSTALL_HEADERS]/QtZlib
    }
}

load(qt_tool)
