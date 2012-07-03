load(qt_build_config)

DESTDIR = $$QT.designer.bins

QT = core-private qmldevtools-private
DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

include(../shared/formats.pri)
include(../shared/proparser.pri)

SOURCES += \
    main.cpp \
    merge.cpp \
    ../shared/simtexth.cpp \
    \
    cpp.cpp \
    java.cpp \
    qdeclarative.cpp \
    ui.cpp

HEADERS += \
    lupdate.h \
    ../shared/simtexth.h

win32:RC_FILE = winmanifest.rc

embed_manifest_exe:win32-msvc2005 {
    # The default configuration embed_manifest_exe overrides the manifest file
    # already embedded via RC_FILE. Vs2008 already have the necessary manifest entry
    QMAKE_POST_LINK += $$quote(mt.exe -updateresource:$$DESTDIR/lupdate.exe -manifest \"$${PWD}\\lupdate.exe.manifest\")
}

load(qt_tool)
