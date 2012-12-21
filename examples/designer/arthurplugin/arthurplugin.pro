QT += designer

QTDIR_build {
# This is only for the Qt build. Do not use externally. We mean it.
PLUGIN_TYPE = designer
PLUGIN_CLASS_NAME = ArthurPlugins
load(qt_plugin)
} else {
# Public example:

CONFIG      += plugin
TEMPLATE    = lib

TARGET = $$qtLibraryTarget($$TARGET)

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

}

qtHaveModule(opengl) {
    DEFINES += QT_OPENGL_SUPPORT
    QT += opengl
}

requires(!isEmpty(_QMAKE_SUPER_CACHE_))
QT_BASE_EXAMPLES = ../../../../qtbase/examples
include($$QT_BASE_EXAMPLES/widgets/painting/shared/shared.pri)

EXAMPLE_AFFINE_DIR = $$QT_BASE_EXAMPLES/widgets/painting/affine
EXAMPLE_COMPOSITION_DIR = $$QT_BASE_EXAMPLES/widgets/painting/composition
EXAMPLE_DEFORM_DIR = $$QT_BASE_EXAMPLES/widgets/painting/deform
EXAMPLE_GRADIENT_DIR = $$QT_BASE_EXAMPLES/widgets/painting/gradients
EXAMPLE_STROKE_DIR = $$QT_BASE_EXAMPLES/widgets/painting/pathstroke

INCLUDEPATH += $$EXAMPLE_AFFINE_DIR $$EXAMPLE_COMPOSITION_DIR $$EXAMPLE_DEFORM_DIR $$EXAMPLE_GRADIENT_DIR $$EXAMPLE_STROKE_DIR

SOURCES = plugin.cpp \
    $$EXAMPLE_AFFINE_DIR/xform.cpp \
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

win32-msvc* {
    QMAKE_CFLAGS += /Zm500
    QMAKE_CXXFLAGS += /Zm500
}

