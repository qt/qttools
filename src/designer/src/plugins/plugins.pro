TEMPLATE = subdirs
CONFIG += ordered

# qtHaveModule(opengl): SUBDIRS += tools/view3d
qtHaveModule(webkitwidgets): SUBDIRS += qwebview
# win32: qtHaveModule(activeqt): SUBDIRS += activeqt
