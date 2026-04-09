TEMPLATE = app

QT += qml quick coap

CONFIG += qmltypes
QML_IMPORT_NAME = CoapSecureClientModule
QML_IMPORT_MAJOR_VERSION = 1

HEADERS += \
    qmlcoapsecureclient.h

SOURCES += \
    main.cpp \
    qmlcoapsecureclient.cpp

qml_resources.files = \
    qmldir \
    FilePicker.qml \
    Main.qml

qml_resources.prefix = /qt/qml/CoapSecureClientModule

RESOURCES += qml_resources

target.path = $$[QT_INSTALL_EXAMPLES]/coap/quicksecureclient
INSTALLS += target
