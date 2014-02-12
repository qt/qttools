win32-msvc2012|win32-msvc2013:equals(QT_ARCH, i386) {
    INCLUDEPATH += $$PWD
    HEADERS += $$PWD/ccapi.h $$PWD/corecon.h
    SOURCES += $$PWD/corecon.cpp
}
