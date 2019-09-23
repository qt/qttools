# Generated from lupdate.pro.

#####################################################################
## lupdate Tool:
#####################################################################

qt_add_tool(lupdate
    TOOLS_TARGET Linguist # special case
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/projectdescriptionreader.cpp ../shared/projectdescriptionreader.h
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/qrcreader.cpp ../shared/qrcreader.h
        ../shared/runqttool.cpp ../shared/runqttool.h
        ../shared/simtexth.cpp ../shared/simtexth.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        cpp.cpp
        java.cpp
        lupdate.h
        main.cpp
        merge.cpp
        ui.cpp
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

#### Keys ignored in scope 1:.:.:lupdate.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Translation File Update Tool"
# QT_TOOL_ENV = "qmake"
# _OPTION = "host_build"
# qmake.name = "QMAKE"
# qmake.value = "$$shell_path($$QMAKE_QMAKE)"

## Scopes:
#####################################################################

qt_extend_target(lupdate CONDITION TARGET Qt::QmlDevToolsPrivate
    SOURCES
        qdeclarative.cpp
    LIBRARIES
        Qt::QmlDevToolsPrivate
    PUBLIC_LIBRARIES
        Qt::QmlDevTools
)

qt_extend_target(lupdate CONDITION NOT TARGET Qt::QmlDevToolsPrivate
    DEFINES
        QT_NO_QML
)

#### Keys ignored in scope 5:.:.:lupdate.pro:mingw:
# RC_FILE = "lupdate.rc"
