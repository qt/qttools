include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(qdbus))

TEMPLATE = subdirs
QT_FOR_CONFIG += xml
qtConfig(dom): SUBDIRS = qdbus
qtHaveModule(widgets) {
    QT_FOR_CONFIG += widgets
    qtConfig(dialogbuttonbox):qtConfig(inputdialog):qtConfig(messagebox):qtConfig(menu) {
        SUBDIRS += qdbusviewer
    }
}
