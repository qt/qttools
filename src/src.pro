TEMPLATE = subdirs

qtHaveModule(widgets) {
    no-png {
        message("Some graphics-related tools are unavailable without PNG support")
    } else {
        SUBDIRS = assistant \
                  pixeltool \
                  designer

        qtHaveModule(quick):qtConfig(thread): SUBDIRS += distancefieldgenerator

        linguist.depends = designer
    }
}

SUBDIRS += linguist \
    qtattributionsscanner

qtConfig(library) {
    !android|android_app: SUBDIRS += qtplugininfo
}

config_clang: qtConfig(thread): SUBDIRS += qdoc

!android|android_app: SUBDIRS += qtpaths

macos {
    SUBDIRS += macdeployqt
}

qtHaveModule(dbus): SUBDIRS += qdbus

win32|winrt:SUBDIRS += windeployqt
winrt:SUBDIRS += winrtrunner
qtHaveModule(gui):!android:!uikit:!qnx:!winrt: SUBDIRS += qtdiag

qtNomakeTools( \
    distancefieldgenerator \
    pixeltool \
)

# This is necessary to avoid a race condition between toolchain.prf
# invocations in a module-by-module cross-build.
cross_compile:isEmpty(QMAKE_HOST_CXX.INCDIRS) {
    qdoc.depends += qtattributionsscanner
    windeployqt.depends += qtattributionsscanner
    winrtrunner.depends += qtattributionsscanner
    linguist.depends += qtattributionsscanner
}
