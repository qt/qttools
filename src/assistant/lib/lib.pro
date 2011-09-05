MODULE = help

load(qt_module)

QT += sql \
    network \
    core-private
TEMPLATE = lib
TARGET = QtHelp

CONFIG += module
MODULE_PRI += ../../../modules/qt_help.pri

DEFINES += QHELP_LIB \
    QT_CLUCENE_SUPPORT
CONFIG += qt \
    warn_on
include($$QT_SOURCE_TREE/src/qbase.pri)

HEADERS += qthelpversion.h

QMAKE_TARGET_PRODUCT = Help
QMAKE_TARGET_DESCRIPTION = Help \
    application \
    framework.
DEFINES -= QT_ASCII_CAST_WARNINGS
INCLUDEPATH += $$QT.help.includes
qclucene = QtCLucene$${QT_LIBINFIX}
if(!debug_and_release|build_pass):CONFIG(debug, debug|release) { 
    mac:qclucene = $${qclucene}_debug
    win32:qclucene = $${qclucene}d
}
linux-lsb-g++:LIBS_PRIVATE += --lsb-shared-libs=$$qclucene
unix|win32-g++*:QMAKE_PKGCONFIG_REQUIRES += QtNetwork \
    QtSql \
    QtXml
LIBS_PRIVATE += -L$$QT.clucene.libs -l$$qclucene
RESOURCES += helpsystem.qrc
SOURCES += qhelpenginecore.cpp \
    qhelpengine.cpp \
    qhelpdbreader.cpp \
    qhelpcontentwidget.cpp \
    qhelpindexwidget.cpp \
    qhelpgenerator.cpp \
    qhelpdatainterface.cpp \
    qhelpprojectdata.cpp \
    qhelpcollectionhandler.cpp \
    qhelpsearchengine.cpp \
    qhelpsearchquerywidget.cpp \
    qhelpsearchresultwidget.cpp \
    qhelpsearchindex_default.cpp \
    qhelpsearchindexwriter_default.cpp \
    qhelpsearchindexreader_default.cpp \
    qhelpsearchindexreader.cpp \
    qclucenefieldnames.cpp \
    qhelp_global.cpp

# access to clucene
SOURCES += qhelpsearchindexwriter_clucene.cpp \
    qhelpsearchindexreader_clucene.cpp
HEADERS += qhelpenginecore.h \
    qhelpengine.h \
    qhelpengine_p.h \
    qhelp_global.h \
    qhelpdbreader_p.h \
    qhelpcontentwidget.h \
    qhelpindexwidget.h \
    qhelpgenerator_p.h \
    qhelpdatainterface_p.h \
    qhelpprojectdata_p.h \
    qhelpcollectionhandler_p.h \
    qhelpsearchengine.h \
    qhelpsearchquerywidget.h \
    qhelpsearchresultwidget.h \
    qhelpsearchindex_default_p.h \
    qhelpsearchindexwriter_default_p.h \
    qhelpsearchindexreader_default_p.h \
    qhelpsearchindexreader_p.h \
    qclucenefieldnames_p.h

# access to clucene
HEADERS += qhelpsearchindexwriter_clucene_p.h \
    qhelpsearchindexreader_clucene_p.h
