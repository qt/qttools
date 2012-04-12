load(qt_build_config)

MODULE = designercomponents
TARGET = QtDesignerComponents
QT = core gui-private widgets-private designer-private

load(qt_module_config)

# private dependencies
QT += xml

# QtDesignerComponents uses
DEFINES += QT_STATICPLUGIN
DEFINES += QDESIGNER_COMPONENTS_LIBRARY

load(qt_targets)
QMAKE_TARGET_PRODUCT = Designer
QMAKE_TARGET_DESCRIPTION = Graphical user interface designer.

#load up the headers info
HEADERS_PRI = $$QT.designer.includes/headers.pri
include($$HEADERS_PRI, "", true)|clear(HEADERS_PRI)

#mac frameworks
mac:!static:contains(QT_CONFIG, qt_framework) {
   QMAKE_FRAMEWORK_BUNDLE_NAME = $$TARGET
   CONFIG += lib_bundle qt_no_framework_direct_includes qt_framework
   CONFIG(debug, debug|release):!build_pass:CONFIG += build_all
}

SOURCES += qdesigner_components.cpp

INCLUDEPATH += . .. \
    $$PWD/../../lib/components \
    $$PWD/../../lib/sdk \
    $$PWD/../../lib/extension \
    $$PWD/../../lib/shared

include(../propertyeditor/propertyeditor.pri)
include(../objectinspector/objectinspector.pri)
include(../signalsloteditor/signalsloteditor.pri)
include(../formeditor/formeditor.pri)
include(../widgetbox/widgetbox.pri)
include(../buddyeditor/buddyeditor.pri)
include(../taskmenu/taskmenu.pri)
include(../tabordereditor/tabordereditor.pri)

PRECOMPILED_HEADER= lib_pch.h

include(../../sharedcomponents.pri)

unix|win32-g++* {
    QMAKE_PKGCONFIG_REQUIRES = QtCore QtDesigner QtGui QtXml
}
