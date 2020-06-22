# Generated from distancefieldgenerator.pro.

#####################################################################
## qdistancefieldgenerator Binary:
#####################################################################

qt_add_executable(qdistancefieldgenerator
    GUI
    SOURCES
        distancefieldmodel.cpp distancefieldmodel.h
        distancefieldmodelworker.cpp distancefieldmodelworker.h
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::QuickPrivate
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)

#### Keys ignored in scope 1:.:.:distancefieldgenerator.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Distance Field Generator"
# QT_FOR_CONFIG = "tools-private"
# _REQUIREMENTS = "qtConfig(distancefieldgenerator)"
qt_add_docs(qdistancefieldgenerator
    doc/qtdistancefieldgenerator.qdocconf
)

