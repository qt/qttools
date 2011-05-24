#! [0]
CONFIG      += uitools

HEADERS     = calculatorform.h
RESOURCES   = calculatorbuilder.qrc
SOURCES     = calculatorform.cpp \
              main.cpp
QT += widgets
#! [0]

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/calculatorbuilder
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.ui *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/calculatorbuilder
INSTALLS += target sources

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example
symbian: warning(This example does not work on Symbian platform)
maemo5: warning(This example does not work on Maemo platform)
simulator: warning(This example does not work on Simulator platform)
