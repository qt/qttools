MODULE = clucene

load(qt_build_config)

TARGET = QtCLucene
QT = core
CONFIG += internal_module

DEFINES += QHELP_LIB

load(qt_module_config)

include(fulltextsearch.pri)

CONFIG += qt warn_off
contains(QT_CONFIG, reduce_exports) {
    CONFIG += hide_symbols
    # workaround for compiler errors on Ubuntu
    linux*-g++*:DEFINES += _GLIBCXX_EXTERN_TEMPLATE=0
}

# impossible to disable exceptions in clucene atm
CONFIG(exceptions_off) {
    CONFIG -= exceptions_off
    CONFIG += exceptions
    !win32|win32-g++* {
        QMAKE_CFLAGS -= -fno-exceptions
        QMAKE_CXXFLAGS -= -fno-exceptions
        QMAKE_LFLAGS -= -fno-exceptions
        QMAKE_CFLAGS += -fexceptions
        QMAKE_CXXFLAGS += -fexceptions
        QMAKE_LFLAGS += -fexceptions
    }
}

win32-msvc.net | win32-msvc2* {
    QMAKE_CFLAGS_RELEASE	-= -O2
    QMAKE_CXXFLAGS_RELEASE	-= -O2    
}

# the following define could be set globally in case we need it elsewhere
solaris* {
    DEFINES += Q_SOLARIS_VERSION=$$system(uname -r | sed -e 's/5\\.//')
}
