TEMPLATE = subdirs

qtHaveModule(widgets): SUBDIRS += simplecoapclient

qtHaveModule(quick) {
    SUBDIRS += \
        quicksecureclient \
        quickmulticastclient
}
