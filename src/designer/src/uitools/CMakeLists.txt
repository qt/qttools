# Generated from uitools.pro.

#####################################################################
## UiTools Module:
#####################################################################

qt_internal_add_module(UiTools
    SOURCES
        ../lib/uilib/abstractformbuilder.cpp ../lib/uilib/abstractformbuilder.h
        ../lib/uilib/formbuilder.cpp ../lib/uilib/formbuilder.h
        ../lib/uilib/formbuilderextra.cpp ../lib/uilib/formbuilderextra_p.h
        ../lib/uilib/properties.cpp ../lib/uilib/properties_p.h
        ../lib/uilib/resourcebuilder.cpp ../lib/uilib/resourcebuilder_p.h
        ../lib/uilib/textbuilder.cpp ../lib/uilib/textbuilder_p.h
        ../lib/uilib/ui4.cpp ../lib/uilib/ui4_p.h
        quiloader.cpp quiloader.h
    DEFINES
        QFORMINTERNAL_NAMESPACE
        QT_DESIGNER
        QT_DESIGNER_STATIC
        QT_USE_QSTRINGBUILDER
    INCLUDE_DIRECTORIES
        ../lib/uilib
    LIBRARIES
        Qt::UiPlugin
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

## Scopes:
#####################################################################

qt_internal_extend_target(UiTools CONDITION TARGET Qt::OpenGLWidgets
    PUBLIC_LIBRARIES
        Qt::OpenGLWidgets
)

qt_internal_extend_target(UiTools CONDITION QT_FEATURE_opengl
    LIBRARIES
        Qt::OpenGL
)
qt_internal_add_docs(UiTools
    doc/qtuitools.qdocconf
)

