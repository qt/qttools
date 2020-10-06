include($$OUT_PWD/../../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(linguist))

TEMPLATE = subdirs
SUBDIRS  = \
    lconvert \
    lprodump \
    lrelease \
    lrelease-pro \
    lupdate \
    lupdate-pro
!no-png:qtHaveModule(widgets) {
    QT_FOR_CONFIG += widgets
    qtConfig(process):qtConfig(pushbutton):qtConfig(toolbutton) {
        SUBDIRS += linguist
    }
}

qtNomakeTools( \
    linguist \
)

load(qt_build_paths)

