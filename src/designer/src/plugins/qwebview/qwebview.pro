TEMPLATE    = lib
TARGET      = qwebview
CONFIG     += qt warn_on plugin
QT         += widgets webkit

include(../plugins.pri)

SOURCES += qwebview_plugin.cpp
HEADERS += qwebview_plugin.h
RESOURCES += qwebview_plugin.qrc
OTHER_FILES += qwebview.json
