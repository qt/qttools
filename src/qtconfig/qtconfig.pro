CONFIG   += x11
QT += gui-private widgets

contains(QT_CONFIG, gstreamer):LIBS += $$QT_LIBS_GSTREAMER -lgstinterfaces-0.10 -lgstvideo-0.10 -lgstbase-0.10
contains(QT_CONFIG, gstreamer):QMAKE_CXXFLAGS += $$QT_CFLAGS_GSTREAMER
contains(QT_CONFIG, phonon) {
    QT += phonon
    DEFINES += HAVE_PHONON
}
SOURCES        += colorbutton.cpp main.cpp previewframe.cpp previewwidget.cpp mainwindow.cpp paletteeditoradvanced.cpp
HEADERS        += colorbutton.h previewframe.h previewwidget.h mainwindow.h paletteeditoradvanced.h

FORMS        = mainwindow.ui paletteeditoradvanced.ui previewwidget.ui
RESOURCES    = qtconfig.qrc

PROJECTNAME        = Qt Configuration

DBFILE          = qtconfig.db

load(qt_app)
