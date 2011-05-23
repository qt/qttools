#! [0]
HEADERS     = calculatorform.h
#! [0] #! [1]
FORMS       = calculatorform.ui
#! [1]
SOURCES     = calculatorform.cpp \
              main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/calculatorform
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/calculatorform
INSTALLS += target sources

symbian: CONFIG += qt_example
maemo5: CONFIG += qt_example

symbian: warning(This example does not work on Symbian platform)
maemo5: warning(This example does not work on Maemo platform)
simulator: warning(This example does not work on Simulator platform)
