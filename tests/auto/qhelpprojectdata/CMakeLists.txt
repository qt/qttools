# Generated from qhelpprojectdata.pro.

#####################################################################
## tst_qhelpprojectdata Test:
#####################################################################

qt_internal_add_test(tst_qhelpprojectdata
    SOURCES
        ../../../src/assistant/qhelpgenerator/qhelpdatainterface.cpp ../../../src/assistant/qhelpgenerator/qhelpdatainterface_p.h
        ../../../src/assistant/qhelpgenerator/qhelpprojectdata.cpp ../../../src/assistant/qhelpgenerator/qhelpprojectdata_p.h
        tst_qhelpprojectdata.cpp
    DEFINES
        QT_USE_USING_NAMESPACE
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::HelpPrivate
)
