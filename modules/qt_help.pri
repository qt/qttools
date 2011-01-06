QT_HELP_VERSION = $$QT_VERSION
QT_HELP_MAJOR_VERSION = $$QT_MAJOR_VERSION
QT_HELP_MINOR_VERSION = $$QT_MINOR_VERSION
QT_HELP_PATCH_VERSION = $$QT_PATCH_VERSION

QT.help.name = QtHelp
QT.help.bins = $$QT_MODULE_BIN_BASE
QT.help.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtHelp
QT.help.private_includes = $$QT_MODULE_INCLUDE_BASE/QtHelp/private
QT.help.sources = $$QT_MODULE_BASE/src/assistant/lib
QT.help.libs = $$QT_MODULE_LIB_BASE
QT.help.depends = network xml sql
QT.help.DEFINES = QT_HELP_LIB
