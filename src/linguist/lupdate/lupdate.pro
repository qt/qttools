DESTDIR = $$QT.designer.bins

QT = core-private

!isEmpty(QT.declarative.name) {
    QT += qmldevtools-private
} else {
    DEFINES += QT_NO_QML
}

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
    ui.cpp

!isEmpty(QT.declarative.name): SOURCES += qdeclarative.cpp

HEADERS += \
    lupdate.h \
    ../shared/simtexth.h

win32:QMAKE_MANIFEST = lupdate.exe.manifest

embed_manifest_exe:win32-msvc2005 {
    # The default configuration embed_manifest_exe overrides the manifest file
    # already embedded via RC_FILE. Vs2008 already have the necessary manifest entry
    QMAKE_POST_LINK += $$quote(mt.exe -updateresource:$$DESTDIR/lupdate.exe -manifest \"$${PWD}\\lupdate.exe.manifest\")
}

load(qt_tool)
