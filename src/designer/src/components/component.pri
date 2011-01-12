load(qt_module)
TARGET = $$qtLibraryTarget($$TARGET$$QT_LIBINFIX) #do this towards the end
INCLUDEPATH += $$QT_SOURCE_TREE/tools/uilib
