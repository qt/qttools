# Generated from lprodump.pro.

#####################################################################
## lprodump Tool:
#####################################################################

add_qt_tool(lprodump
    SOURCES
        ../shared/ioutils.cpp ../shared/ioutils.h
        ../shared/profileevaluator.cpp ../shared/profileevaluator.h
        ../shared/proitems.cpp ../shared/proitems.h
        ../shared/qmake_global.h
        ../shared/qmakebuiltins.cpp
        ../shared/qmakeevaluator.cpp ../shared/qmakeevaluator.h ../shared/qmakeevaluator_p.h
        ../shared/qmakeglobals.cpp ../shared/qmakeglobals.h
        ../shared/qmakeparser.cpp ../shared/qmakeparser.h
        ../shared/qmakevfs.cpp ../shared/qmakevfs.h
        ../shared/qrcreader.cpp ../shared/qrcreader.h
        main.cpp
    DEFINES
        PROEVALUATOR_CUMULATIVE
        PROEVALUATOR_DEBUG
        PROEVALUATOR_INIT_PROPS
        QMAKE_BUILTIN_PRFS
        QMAKE_OVERRIDE_PRFS
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
)

# Resources:
set_source_files_properties("../shared/exclusive_builds.prf"
    PROPERTIES QT_RESOURCE_ALIAS "exclusive_builds.prf"
)
set(proparser_resource_files
    "exclusive_builds.prf"
)

add_qt_resource(lprodump "proparser"
    PREFIX
        "/qmake/override_features"
    BASE
        "../shared"
    FILES
        ${proparser_resource_files}
)


#### Keys ignored in scope 1:.:.:lprodump.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Linguist QMake Project Dump Tool"
# QT_TOOL_ENV = "qmake"
# _LOADED = "qt_tool"
# _OPTION = "host_build"
# qmake.name = "QMAKE"
# qmake.value = "$$shell_path($$QMAKE_QMAKE)"
