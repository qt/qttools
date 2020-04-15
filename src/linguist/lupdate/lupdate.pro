option(host_build)
QT = core-private tools-private

# Needed to get access to all the CLANG_FOO assignments.
include($$OUT_PWD/../../../src/global/qttools-config.pri)

qtHaveModule(qmldevtools-private) {
    QT += qmldevtools-private
} else {
    DEFINES += QT_NO_QML
}

qtConfig(clangcpp) {
    LIBS += $$CLANGCPP_LIBS $$CLANG_LIBS

    !contains(QMAKE_DEFAULT_INCDIRS, $$CLANG_INCLUDEPATH): INCLUDEPATH += $$CLANG_INCLUDEPATH
    DEFINES += $$CLANG_DEFINES

    !contains(QMAKE_DEFAULT_LIBDIRS, $$CLANG_LIBDIR):!disable_external_rpath: QMAKE_RPATHDIR += $$CLANG_LIBDIR
    DEFINES += $$shell_quote(CLANG_RESOURCE_DIR=\"$${CLANG_LIBDIR}/clang/$${CLANG_VERSION}/include\")

    DEFINES += $$shell_quote(LUPDATE_CLANG_VERSION_STR=\"$${CLANG_VERSION}\") \
        LUPDATE_CLANG_VERSION_MAJOR=$${CLANG_MAJOR_VERSION} \
        LUPDATE_CLANG_VERSION_MINOR=$${CLANG_MINOR_VERSION} \
        LUPDATE_CLANG_VERSION_PATCH=$${CLANG_PATCH_VERSION}
}

CONFIG += rtti_off

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

include(../shared/formats.pri)

SOURCES += \
    main.cpp \
    merge.cpp \
    ../shared/projectdescriptionreader.cpp \
    ../shared/runqttool.cpp \
    ../shared/qrcreader.cpp \
    ../shared/simtexth.cpp \
    cpp.cpp \
    java.cpp \
    ui.cpp

qtHaveModule(qmldevtools-private): SOURCES += qdeclarative.cpp

HEADERS += \
    lupdate.h \
    cpp.h \
    ../shared/projectdescriptionreader.h \
    ../shared/qrcreader.h \
    ../shared/runqttool.h \
    ../shared/simtexth.h

qtConfig(clangcpp) {
    SOURCES += \
        cpp_clang.cpp \
        clangtoolastreader.cpp \
        lupdatepreprocessoraction.cpp
    HEADERS += \
        cpp_clang.h \
        clangtoolastreader.h \
        lupdatepreprocessoraction.h \
        synchronized.h
}

mingw {
    RC_FILE = lupdate.rc
}

qmake.name = QMAKE
qmake.value = $$shell_path($$QMAKE_QMAKE)
QT_TOOL_ENV += qmake

QMAKE_TARGET_DESCRIPTION = "Qt Translation File Update Tool"
load(qt_tool)
