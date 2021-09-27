# Generated from lconvert.pro.

#####################################################################
## lconvert Tool:
#####################################################################

qt_get_tool_target_name(target_name lconvert)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Converter"
    TOOLS_TARGET Linguist # special case
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        ../shared/xmlparser.cpp ../shared/xmlparser.h
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    PUBLIC_LIBRARIES
        Qt::CorePrivate
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:lconvert.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Translation File Converter"
# _OPTION = "host_build"
