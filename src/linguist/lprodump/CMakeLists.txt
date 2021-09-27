# Generated from lprodump.pro.

#####################################################################
## lprodump Tool:
#####################################################################

qt_get_tool_target_name(target_name lprodump)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Linguist QMake Project Dump Tool"
    TOOLS_TARGET Linguist # special case
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
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
qt_internal_return_unless_building_tools()

# Resources:
set(proparser_resource_files
    "../shared/exclusive_builds.prf"
)

qt_internal_add_resource(${target_name} "proparser"
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
# _OPTION = "host_build"
# qmake.name = "QMAKE"
# qmake.value = "$$shell_path($$QMAKE_QMAKE)"
