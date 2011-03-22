QT_CLUCENE_VERSION = $$QT_VERSION
QT_CLUCENE_MAJOR_VERSION = $$QT_MAJOR_VERSION
QT_CLUCENE_MINOR_VERSION = $$QT_MINOR_VERSION
QT_CLUCENE_PATCH_VERSION = $$QT_PATCH_VERSION

QT.clucene.name = QtCLucene
QT.clucene.bins = $$QT_MODULE_BIN_BASE
QT.clucene.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtCLucene
QT.clucene.private_includes = $$QT_MODULE_INCLUDE_BASE/QtCLucene/private
QT.clucene.sources = $$QT_MODULE_BASE/src/assistant/lib/fulltextsearch
QT.clucene.libs = $$QT_MODULE_LIB_BASE
QT.clucene.plugins = $$QT_MODULE_PLUGIN_BASE
QT.clucene.imports = $$QT_MODULE_IMPORT_BASE
QT.clucene.depends = core
