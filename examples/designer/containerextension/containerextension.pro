#! [0]
TEMPLATE = lib
CONFIG  += plugin
#! [0]

TARGET   = $$qtLibraryTarget($$TARGET)

#! [3]
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
#! [3]

#! [1]
QT      += widgets designer
#! [1]

#! [2]
HEADERS += multipagewidget.h \
           multipagewidgetplugin.h \
           multipagewidgetcontainerextension.h \
           multipagewidgetextensionfactory.h

SOURCES += multipagewidget.cpp \
           multipagewidgetplugin.cpp \
           multipagewidgetcontainerextension.cpp \
           multipagewidgetextensionfactory.cpp

OTHER_FILES += multipagewidget.json
#! [2]
