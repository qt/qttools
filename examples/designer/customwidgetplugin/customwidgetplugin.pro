#! [0] #! [1]
CONFIG      += designer plugin
QT          += widgets
#! [0]
TARGET      = $$qtLibraryTarget($$TARGET)
#! [2]
TEMPLATE    = lib
#! [1] #! [2]
DESTDIR = $$QT.designer.plugins/designer

#! [3]
HEADERS     = analogclock.h \
              customwidgetplugin.h
SOURCES     = analogclock.cpp \
              customwidgetplugin.cpp
OTHER_FILES += analogclock.json
#! [3]

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/customwidgetplugin
INSTALLS += target sources
