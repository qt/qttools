TEMPLATE = subdirs
qtHaveModule(widgets): SUBDIRS += help designer linguist uitools

!qtConfig(process): SUBDIRS -= designer
