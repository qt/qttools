
CONFIG      += designer plugin
TEMPLATE    = lib
TARGET = $$qtLibraryTarget(arthurplugin)
DESTDIR = $$QT.designer.plugins/designer

contains(QT_CONFIG, opengl) {
    DEFINES += QT_OPENGL_SUPPORT
    QT += opengl
}

SHARED_FOLDER = QT_CORE_SOURCES/examples/painting/shared
include(QT_CORE_SOURCES/examples/painting/shared/shared.pri)

EXAMPLE_AFFINE_DIR = QT_CORE_SOURCES/examples/painting/affine
EXAMPLE_COMPOSITION_DIR = QT_CORE_SOURCES/examples/painting/composition
EXAMPLE_DEFORM_DIR = QT_CORE_SOURCES/examples/painting/deform
EXAMPLE_GRADIENT_DIR = QT_CORE_SOURCES/examples/painting/gradients
EXAMPLE_STROKE_DIR = QT_CORE_SOURCES/examples/painting/pathstroke

INCLUDEPATH += $$EXAMPLE_AFFINE_DIR $$EXAMPLE_COMPOSITION_DIR $$EXAMPLE_DEFORM_DIR $$EXAMPLE_GRADIENT_DIR $$EXAMPLE_STROKE_DIR

SOURCES = plugin.cpp \
    $EXAMPLE_AFFINE_DIR/xform.cpp \
    $$EXAMPLE_COMPOSITION_DIR/composition.cpp \
    $$EXAMPLE_DEFORM_DIR/pathdeform.cpp \
    $$EXAMPLE_GRADIENT_DIR/gradients.cpp \
    $$EXAMPLE_STROKE_DIR/pathstroke.cpp \


HEADERS = \
    $$EXAMPLE_AFFINE_DIR/xform.h \
    $$EXAMPLE_COMPOSITION_DIR/composition.h \
    $$EXAMPLE_DEFORM_DIR/pathdeform.h \
    $$EXAMPLE_GRADIENT_DIR/gradients.h \
    $$EXAMPLE_STROKE_DIR/pathstroke.h \

RESOURCES += arthur_plugin.qrc

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.jpg *.png
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/arthurplugin
INSTALLS += target sources

win32-msvc* {
    QMAKE_CFLAGS += /Zm500
    QMAKE_CXXFLAGS += /Zm500
}

