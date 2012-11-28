TEMPLATE   = app
QT        += widgets

HEADERS   += remotecontrol.h
SOURCES   += main.cpp \
             remotecontrol.cpp
FORMS     += remotecontrol.ui
RESOURCES += remotecontrol.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qttools/help/remotecontrol
INSTALLS += target

