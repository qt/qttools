TEMPLATE = subdirs
CONFIG += ordered

REQUIRES = !CONFIG(static,shared|static)
# qtHaveModule(opengl): SUBDIRS += tools/view3d
qtHaveModule(webkitwidgets): SUBDIRS += qwebview
# win32: contains(QT_CONFIG, activeqt): SUBDIRS += activeqt
