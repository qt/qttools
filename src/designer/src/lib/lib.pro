MODULE = designer

load(qt_module)

TARGET = QtDesigner
QPRO_PWD = $$PWD
QT += core-private gui-private widgets widgets-private xml

CONFIG += module
MODULE_PRI = ../../../../modules/qt_designer.pri

unix|win32-g++*:QMAKE_PKGCONFIG_REQUIRES += QtXml QtCore QtGui QtWidgets

load(qt_module_config)

HEADERS += qtdesignerversion.h

DEFINES += \
    QDESIGNER_SDK_LIBRARY \
    QDESIGNER_EXTENSION_LIBRARY \
    QDESIGNER_UILIB_LIBRARY \
    QDESIGNER_SHARED_LIBRARY

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
include(uilib/uilib.pri)
PRECOMPILED_HEADER=lib_pch.h

include(../sharedcomponents.pri)
