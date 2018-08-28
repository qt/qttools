TEMPLATE = subdirs
qtHaveModule(widgets) {
    SUBDIRS += linguist
    !uikit: SUBDIRS += help designer uitools assistant
}

!qtConfig(process): SUBDIRS -= assistant designer
