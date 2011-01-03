QT_DESIGNER_VERSION = $$QT_VERSION
QT_DESIGNER_MAJOR_VERSION = $$QT_MAJOR_VERSION
QT_DESIGNER_MINOR_VERSION = $$QT_MINOR_VERSION
QT_DESIGNER_PATCH_VERSION = $$QT_PATCH_VERSION

QT.designer.name = QtDesigner
QT.designer.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtDesigner
QT.designer.private_includes = $$QT_MODULE_INCLUDE_BASE/QtDesigner/private
QT.designer.sources = $$QT_MODULE_BASE/src/designer/src/lib
QT.designer.libs = $$QT_MODULE_LIB_BASE
QT.designer.depends = xml
QT.designer.DEFINES = QT_DESIGNER_LIB
