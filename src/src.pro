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

SUBDIRS += linguist \
           qtpaths

mac {
    SUBDIRS += macdeployqt
}

android {
    SUBDIRS += androiddeployqt
}

qtHaveModule(dbus): SUBDIRS += qdbus

qtNomakeTools( \
    pixeltool \
    qtconfig \
    macdeployqt \
)
