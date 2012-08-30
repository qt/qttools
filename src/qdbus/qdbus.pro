TEMPLATE = subdirs
SUBDIRS = qdbus
!isEmpty(QT.widgets.name): SUBDIRS += qdbusviewer
