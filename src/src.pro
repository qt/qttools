TEMPLATE = subdirs
CONFIG += ordered

qtHaveModule(widgets) {
    no-png {
        message("Some graphics-related tools are unavailable without PNG support")
    } else {
        SUBDIRS = assistant \
                  pixeltool \
                  qtestlib \
                  designer
#    unix:!mac:!embedded:!qpa:SUBDIRS += qtconfig
    }
}

SUBDIRS += linguist

mac {
    SUBDIRS += macdeployqt
}

qtHaveModule(dbus): SUBDIRS += qdbus
