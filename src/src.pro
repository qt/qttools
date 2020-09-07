TEMPLATE = subdirs

qtHaveModule(widgets) {
    no-png {
        message("Some graphics-related tools are unavailable without PNG support")
    } else {
        QT_FOR_CONFIG += widgets
        qtConfig(pushbutton):qtConfig(toolbutton) {
            SUBDIRS = designer \
                      pixeltool

            !static|contains(QT_PLUGINS, qsqlite): SUBDIRS += assistant

            linguist.depends = designer
        }
        qtHaveModule(quick):qtConfig(thread):qtConfig(toolbutton): SUBDIRS += distancefieldgenerator
    }
}

SUBDIRS += linguist

qtConfig(commandlineparser) {
    SUBDIRS += qtattributionsscanner
    qtConfig(library) {
        !android|android_app: SUBDIRS += qtplugininfo
    }
}


SUBDIRS += global
include($$OUT_PWD/../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
qtConfig(clang): qtConfig(thread): qtConfig(commandlineparser): SUBDIRS += qdoc

qtConfig(commandlineparser): !android|android_app: SUBDIRS += qtpaths

macos {
    SUBDIRS += macdeployqt
}

qtHaveModule(dbus): SUBDIRS += qdbus

win32:SUBDIRS += windeployqt
qtHaveModule(gui):qtConfig(commandlineparser):!wasm:!android:!uikit:!qnx: SUBDIRS += qtdiag

qtNomakeTools( \
    distancefieldgenerator \
    pixeltool \
)

# This is necessary to avoid a race condition between toolchain.prf
# invocations in a module-by-module cross-build.
cross_compile:isEmpty(QMAKE_HOST_CXX.INCDIRS) {
    qdoc.depends += qtattributionsscanner
    windeployqt.depends += qtattributionsscanner
    linguist.depends += qtattributionsscanner
}
