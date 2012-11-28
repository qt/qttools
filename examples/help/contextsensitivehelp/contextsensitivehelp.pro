TEMPLATE = app

QT     += help widgets
SOURCES += main.cpp \
           wateringconfigdialog.cpp \
           helpbrowser.cpp

HEADERS += wateringconfigdialog.h \
           helpbrowser.h

FORMS   += wateringconfigdialog.ui

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qttools/help/contextsensitivehelp
INSTALLS += target
