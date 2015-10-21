TEMPLATE = subdirs

qtHaveModule(widgets) {
    no-png {
        message("Some graphics-related tools are unavailable without PNG support")
    } else {
        SUBDIRS = assistant \
                  pixeltool \
                  qtestlib \
                  designer

        linguist.depends = designer
    }
}

SUBDIRS += linguist \
    qdoc \
    qtplugininfo
if(!android|android_app):!ios: SUBDIRS += qtpaths

mac {
    SUBDIRS += macdeployqt
}

android {
    SUBDIRS += androiddeployqt
}

qtHaveModule(dbus): SUBDIRS += qdbus

win32|winrt:SUBDIRS += windeployqt
winrt:SUBDIRS += winrtrunner
qtHaveModule(gui):!android:!ios:!qnx:!wince*:!winrt*:SUBDIRS += qtdiag

qtNomakeTools( \
    pixeltool \
    macdeployqt \
)
