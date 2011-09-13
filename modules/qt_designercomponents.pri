QT.designercomponents.VERSION = 5.0.0
QT.designercomponents.MAJOR_VERSION = 5
QT.designercomponents.MINOR_VERSION = 0
QT.designercomponents.PATCH_VERSION = 0

QT.designercomponents.name = QtDesignerComponents
QT.designercomponents.bins = $$QT_MODULE_BIN_BASE
QT.designercomponents.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtDesignerComponents
QT.designercomponents.private_includes = $$QT_MODULE_INCLUDE_BASE/QtDesignerComponents/$$QT.designercomponents.VERSION
QT.designercomponents.sources = $$QT_MODULE_BASE/src/designer/src/components/lib
QT.designercomponents.libs = $$QT_MODULE_LIB_BASE
QT.designercomponents.plugins = $$QT_MODULE_PLUGIN_BASE
QT.designercomponents.imports = $$QT_MODULE_IMPORT_BASE
QT.designercomponents.depends = core xml gui widgets

QT_CONFIG += designercomponents
