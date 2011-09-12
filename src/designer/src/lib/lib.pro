load(qt_module)

TARGET = QtDesigner
QT += core-private gui-private widgets widgets-private xml uilib uilib-private

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

#win32|mac: CONFIG += debug_and_release
#DESTDIR = $$QT_BUILD_TREE/lib
#!wince*:DLLDESTDIR = $$QT.designer.bins

#INCLUDEPATH += $$QT.designer.includes \
#               $$QT.designer.private_includes \
#               $$QT.designer.private_includes/QtDesigner

#isEmpty(QT_MAJOR_VERSION) {
#   VERSION=4.3.0
#} else {
#   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
#}

unix|win32-g++*:QMAKE_PKGCONFIG_REQUIRES += QtXml

load(qt_module_config)

HEADERS += qtdesignerversion.h

!contains(CONFIG, static) {
    CONFIG += dll

    DEFINES += \
        QDESIGNER_SDK_LIBRARY \
        QDESIGNER_EXTENSION_LIBRARY \
        QDESIGNER_UILIB_LIBRARY \
        QDESIGNER_SHARED_LIBRARY
} else {
    DEFINES += QT_DESIGNER_STATIC
}

##headers.pri is loaded from the last include path
#LAST_MODULE_INCLUDE=$$INCLUDEPATH
#for(include_path, INCLUDEPATH):LAST_MODULE_INCLUDE=$${include_path}
#HEADERS_PRI = $$LAST_MODULE_INCLUDE/headers.pri
#include($$HEADERS_PRI, "", true)|clear(HEADERS_PRI)

#mac frameworks
mac:CONFIG += explicitlib
mac:!static:contains(QT_CONFIG, qt_framework) {
   QMAKE_FRAMEWORK_BUNDLE_NAME = $$TARGET
   CONFIG += lib_bundle qt_no_framework_direct_includes qt_framework
   CONFIG(debug, debug|release) {
      !build_pass:CONFIG += build_all
   } else { #release
      !debug_and_release|build_pass {
	  CONFIG -= qt_install_headers #no need to install these as well
	  FRAMEWORK_HEADERS.version = Versions
	  FRAMEWORK_HEADERS.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
      	  FRAMEWORK_HEADERS.path = Headers
      }
      QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
   }
}

include(extension/extension.pri)
include(sdk/sdk.pri)
include(shared/shared.pri)
PRECOMPILED_HEADER=lib_pch.h

include(../sharedcomponents.pri)
include(../components/component.pri)

#target.path=$$[QT_INSTALL_LIBS]
#INSTALLS        += target
#win32 {
#    dlltarget.path=$$[QT_INSTALL_BINS]
#    INSTALLS += dlltarget
#}


#qt_install_headers {
#    designer_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
#    designer_headers.path = $$[QT_INSTALL_HEADERS]/QtDesigner
#    INSTALLS        += designer_headers
#}
