QT.help.VERSION = 5.0.0
QT.help.MAJOR_VERSION = 5
QT.help.MINOR_VERSION = 0
QT.help.PATCH_VERSION = 0

QT.help.name = QtHelp
QT.help.bins = $$QT_MODULE_BIN_BASE
QT.help.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtHelp
QT.help.private_includes = $$QT_MODULE_INCLUDE_BASE/QtHelp/$$QT.help.VERSION
QT.help.sources = $$QT_MODULE_BASE/src/assistant/lib
QT.help.libs = $$QT_MODULE_LIB_BASE
QT.help.plugins = $$QT_MODULE_PLUGIN_BASE
QT.help.imports = $$QT_MODULE_IMPORT_BASE
QT.help.depends = network xml sql
QT.help.DEFINES = QT_HELP_LIB
