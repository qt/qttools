QPRO_PWD   = $$PWD
TEMPLATE = lib
TARGET = $$qtLibraryTarget(QtUiTools)
QT = core

CONFIG += qt staticlib module
MODULE = uitools
MODULE_PRI = ../modules/qt_uitools.pri 

DESTDIR = $$QMAKE_LIBDIR_QT

win32|mac:!macx-xcode:CONFIG += debug_and_release build_all

DEFINES += QFORMINTERNAL_NAMESPACE QT_DESIGNER_STATIC QT_FORMBUILDER_NO_SCRIPT
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.3.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
load(qt_targets)
QMAKE_TARGET_PRODUCT = UiLoader
QMAKE_TARGET_DESCRIPTION = QUiLoader

include(../lib/uilib/uilib.pri)

HEADERS += quiloader.h
SOURCES += quiloader.cpp

include($$QT_BUILD_TREE/include/QtUiTools/headers.pri, "", true)
quitools_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
quitools_headers.path = $$[QT_INSTALL_HEADERS]/QtUiTools
quitools_private_headers.files = $$SYNCQT.PRIVATE_HEADER_FILES
quitools_private_headers.path = $$[QT_INSTALL_HEADERS]/QtUiTools/$$QT.uitools.VERSION/QtUiTools/private
INSTALLS        += quitools_headers quitools_private_headers

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

unix|win32-g++* {
   CONFIG     += create_pc
   QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]
   QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]/$$TARGET
   QMAKE_PKGCONFIG_CFLAGS = -I$$[QT_INSTALL_HEADERS]
   QMAKE_PKGCONFIG_DESTDIR = pkgconfig
}
