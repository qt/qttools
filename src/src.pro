TEMPLATE = subdirs

qtHaveModule(widgets) {
    no-png {
        message("Some graphics-related tools are unavailable without PNG support")
    } else {
        SUBDIRS = assistant \
                  pixeltool \
                  qtestlib \
                  designer
#    unix:!mac:!embedded:!qpa:SUBDIRS += qtconfig

        linguist.depends = designer
    }
}

SUBDIRS += linguist
!android|android_app: SUBDIRS += qtpaths

mac {
    SUBDIRS += macdeployqt
}

android {
    SUBDIRS += androiddeployqt
}

qtHaveModule(dbus): SUBDIRS += qdbus

win32|winrt:SUBDIRS += windeployqt
winrt:SUBDIRS += winrtrunner qtd3dservice

qtNomakeTools( \
    pixeltool \
    qtconfig \
    macdeployqt \
)

!android:!ios:!qnx:!wince*:!winrt*:SUBDIRS += qtdiag
