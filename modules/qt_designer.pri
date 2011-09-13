QT.designer.VERSION = 5.0.0
QT.designer.MAJOR_VERSION = 5
QT.designer.MINOR_VERSION = 0
QT.designer.PATCH_VERSION = 0

QT.designer.name = QtDesigner
QT.designer.bins = $$QT_MODULE_BIN_BASE
QT.designer.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtDesigner
QT.designer.private_includes = $$QT_MODULE_INCLUDE_BASE/QtDesigner/$$QT.designer.VERSION
QT.designer.sources = $$QT_MODULE_BASE/src/designer/src/lib
QT.designer.libs = $$QT_MODULE_LIB_BASE
QT.designer.plugins = $$QT_MODULE_PLUGIN_BASE
QT.designer.imports = $$QT_MODULE_IMPORT_BASE
QT.designer.depends = core xml gui widgets
QT.designer.DEFINES = QT_DESIGNER_LIB

QT_CONFIG += designer
