CONFIG += designer
win32|mac: CONFIG+= debug_and_release
DESTDIR = $$QT.designer.plugins/designer
contains(TEMPLATE, ".*lib"):TARGET = $$qtLibraryTarget($$TARGET)

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

INCLUDEPATH += $$QT.designer.includes
