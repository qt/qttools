# Generated from lconvert.pro.

#####################################################################
## lconvert Tool:
#####################################################################

add_qt_tool(lconvert
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        main.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Xml
)

#### Keys ignored in scope 1:.:.:lconvert.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Translation File Converter"
# _LOADED = "qt_tool"
# _OPTION = "host_build"
