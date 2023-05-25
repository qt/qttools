#! [1]
QT          += widgets uiplugin
#! [1]

#! [0]
CONFIG      += plugin
TEMPLATE    = lib
#! [0]

#! [3]
TARGET = $$qtLibraryTarget($$TARGET)

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
#! [3]

#! [2]
HEADERS     = analogclock.h \
              customwidgetplugin.h
SOURCES     = analogclock.cpp \
              customwidgetplugin.cpp
OTHER_FILES += analogclock.json
#! [2]
