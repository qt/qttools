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
