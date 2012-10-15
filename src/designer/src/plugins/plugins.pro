TEMPLATE = subdirs
CONFIG += ordered

REQUIRES = !CONFIG(static,shared|static)
# contains(QT_CONFIG, opengl): SUBDIRS += tools/view3d
contains(QT_CONFIG, webkitwidgets): SUBDIRS += qwebview
# win32: contains(QT_CONFIG, activeqt): SUBDIRS += activeqt
