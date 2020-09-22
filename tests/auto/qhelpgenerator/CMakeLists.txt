# Generated from qhelpgenerator.pro.

#####################################################################
## tst_qhelpgenerator Test:
#####################################################################

qt_internal_add_test(tst_qhelpgenerator
    SOURCES
        ../../../src/assistant/qhelpgenerator/helpgenerator.cpp ../../../src/assistant/qhelpgenerator/helpgenerator.h
        ../../../src/assistant/qhelpgenerator/qhelpdatainterface.cpp ../../../src/assistant/qhelpgenerator/qhelpdatainterface_p.h
        ../../../src/assistant/qhelpgenerator/qhelpprojectdata.cpp ../../../src/assistant/qhelpgenerator/qhelpprojectdata_p.h
        tst_qhelpgenerator.cpp
    DEFINES
        QT_USE_USING_NAMESPACE
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::HelpPrivate
        Qt::Sql
)
