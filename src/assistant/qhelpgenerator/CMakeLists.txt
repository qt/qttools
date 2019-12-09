# Generated from qhelpgenerator.pro.

#####################################################################
## qhelpgenerator Tool:
#####################################################################

qt_add_tool(qhelpgenerator
    SOURCES
        ../shared/collectionconfiguration.cpp ../shared/collectionconfiguration.h
        collectionconfigreader.cpp collectionconfigreader.h
        helpgenerator.cpp helpgenerator.h
        main.cpp
        qhelpdatainterface.cpp qhelpdatainterface_p.h
        qhelpprojectdata.cpp qhelpprojectdata_p.h
    LIBRARIES
        Qt::HelpPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Help
        Qt::Network
)

#### Keys ignored in scope 1:.:.:qhelpgenerator.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Compressed Help File Generator"
# QTPLUGIN.platforms = "qminimal"
