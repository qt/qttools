HEADERS       = mainwindow.h \
                findfiledialog.h \
                assistant.h \
                textedit.h
SOURCES       = main.cpp \
                mainwindow.cpp \
                findfiledialog.cpp \
                assistant.cpp \
                textedit.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qttools/help/simpletextviewer
sources.files = $$SOURCES $$HEADERS $$RESOURCES documentation *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/help/simpletextviewer
INSTALLS += target sources

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example

symbian: warning(This example does not work on Symbian platform)
maemo5: warning(This example does not work on Maemo platform)
simulator: warning(This example does not work on Simulator platform)
