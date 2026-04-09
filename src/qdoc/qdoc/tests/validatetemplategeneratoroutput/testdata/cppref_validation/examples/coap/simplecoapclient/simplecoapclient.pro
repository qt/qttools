QT       += core network coap widgets

TARGET = simplecoapclient
TEMPLATE = app

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        optiondialog.cpp

HEADERS += \
        mainwindow.h \
        optiondialog.h

FORMS += \
        mainwindow.ui \
        optiondialog.ui

target.path = $$[QT_INSTALL_EXAMPLES]/coap/simplecoapclient
INSTALLS += target
