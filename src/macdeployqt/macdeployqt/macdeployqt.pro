DESTDIR     = $$QT.designer.bins

SOURCES += main.cpp ../shared/shared.cpp
CONFIG -= app_bundle

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
