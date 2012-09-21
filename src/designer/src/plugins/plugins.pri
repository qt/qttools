CONFIG += designer
!build_pass:contains(QT_CONFIG, build_all): CONFIG += release
DESTDIR = $$QT.designer.plugins/designer
contains(TEMPLATE, ".*lib"):TARGET = $$qtLibraryTarget($$TARGET)

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

INCLUDEPATH += $$QT.designer.includes
