# Generated from uitools.pro.

#####################################################################
## UiTools Module:
#####################################################################

add_qt_module(UiTools
    STATIC
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

#### Keys ignored in scope 1:.:.:uitools.pro:<TRUE>:
# _LOADED = "qt_module"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:uitools.pro:NOT qtBuildPart(tools):
# MODULE_PLUGIN_TYPES = "designer"
add_qt_docs(
    doc/qtuitools.qdocconf
)

