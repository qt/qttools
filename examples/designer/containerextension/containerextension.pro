#! [0]
TEMPLATE = lib
#! [0]
TARGET   = $$qtLibraryTarget($$TARGET)
#! [1]
CONFIG  += designer plugin
QT      += widgets
#! [1]
DESTDIR = $$QT.designer.plugins/designer

#! [2]
HEADERS += multipagewidget.h \
           multipagewidgetplugin.h \ 
           multipagewidgetcontainerextension.h \
           multipagewidgetextensionfactory.h 

SOURCES += multipagewidget.cpp \
           multipagewidgetplugin.cpp \
           multipagewidgetcontainerextension.cpp \
           multipagewidgetextensionfactory.cpp 
#! [2]

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/containerextension
INSTALLS += target sources

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example
symbian: warning(This example does not work on Symbian platform)
maemo5: warning(This example does not work on Maemo platform)
simulator: warning(This example does not work on Simulator platform)
