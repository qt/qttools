QT.clucene.VERSION = 5.0.0
QT.clucene.MAJOR_VERSION = 5
QT.clucene.MINOR_VERSION = 0
QT.clucene.PATCH_VERSION = 0

QT.clucene.name = QtCLucene
QT.clucene.bins = $$QT_MODULE_BIN_BASE
QT.clucene.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtCLucene
QT.clucene.private_includes = $$QT_MODULE_INCLUDE_BASE/QtCLucene/$$QT.clucene.VERSION
QT.clucene.sources = $$QT_MODULE_BASE/src/assistant/lib/fulltextsearch
QT.clucene.libs = $$QT_MODULE_LIB_BASE
QT.clucene.plugins = $$QT_MODULE_PLUGIN_BASE
QT.clucene.imports = $$QT_MODULE_IMPORT_BASE
QT.clucene.depends = core
