TEMPLATE = app

QT += qml quick coap

CONFIG += qmltypes
QML_IMPORT_NAME = CoapClientModule
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += \
        main.cpp \
        qmlcoapmulticastclient.cpp

HEADERS += \
    qmlcoapmulticastclient.h

qml_resources.files = \
    qmldir \
    Main.qml

qml_resources.prefix = /qt/qml/CoapClientModule

RESOURCES += qml_resources

target.path = $$[QT_INSTALL_EXAMPLES]/coap/quickmulticastclient
INSTALLS += target
