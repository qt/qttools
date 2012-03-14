TEMPLATE = subdirs
SUBDIRS = qdbus
!contains(QT_CONFIG, no-gui): SUBDIRS += qdbusviewer
