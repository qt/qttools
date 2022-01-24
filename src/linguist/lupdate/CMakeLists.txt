# Generated from lupdate.pro.

# special case begin
if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    message(WARNING "lupdate will not be built in this configuration.")
    return()
endif()

if (MINGW)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY _qt_skip_separate_debug_info ON)
endif()
# special case end

#####################################################################
## lupdate Tool:
#####################################################################

qt_get_tool_target_name(target_name lupdate)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Translation File Update Tool"
    TOOLS_TARGET Linguist # special case
    EXTRA_CMAKE_FILES "${CMAKE_CURRENT_LIST_DIR}/../GenerateLUpdateProject.cmake"
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
        ../shared/xmlparser.cpp ../shared/xmlparser.h
        cpp.cpp cpp.h
        java.cpp
        python.cpp
        lupdate.h
        main.cpp
        merge.cpp
        ui.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Tools
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:lupdate.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Translation File Update Tool"
# QT_TOOL_ENV = "qmake"
# _OPTION = "host_build"
# qmake.name = "QMAKE"
# qmake.value = "$$shell_path($$QMAKE_QMAKE)"

## Scopes:
#####################################################################

qt_internal_extend_target(${target_name} CONDITION TARGET Qt::QmlPrivate
    SOURCES
        qdeclarative.cpp
    LIBRARIES
        Qt::QmlPrivate
)

qt_internal_extend_target(${target_name} CONDITION NOT TARGET Qt::QmlPrivate
    DEFINES
        QT_NO_QML
)

qt_internal_extend_target(${target_name} CONDITION QT_FEATURE_clangcpp
    SOURCES
        clangtoolastreader.cpp clangtoolastreader.h
        cpp_clang.cpp cpp_clang.h
        filesignificancecheck.cpp filesignificancecheck.h
        lupdatepreprocessoraction.cpp lupdatepreprocessoraction.h
        synchronized.h
    DEFINES
        # special case begin
        # remove these
        #(CLANG_RESOURCE_DIR=\"/clang//include\")
        #(LUPDATE_CLANG_VERSION_STR=\"\")
        #LUPDATE_CLANG_VERSION_MAJOR=
        #LUPDATE_CLANG_VERSION_MINOR=
        #LUPDATE_CLANG_VERSION_PATCH=
        # in favor of these
        LUPDATE_CLANG_VERSION_STR="\"${QT_LIB_CLANG_VERSION}\""
        LUPDATE_CLANG_VERSION_MAJOR=${QT_LIB_CLANG_VERSION_MAJOR}
        LUPDATE_CLANG_VERSION_MINOR=${QT_LIB_CLANG_VERSION_MINOR}
        LUPDATE_CLANG_VERSION_PATCH=${QT_LIB_CLANG_VERSION_PATCH}
        # special case end
    LIBRARIES # special case
        WrapLibClang::WrapLibClang # special case
)

qt_internal_extend_target(${target_name} CONDITION MSVC
    DEFINES _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

# special case begin
if(QT_FEATURE_clangcpp)
    set_property(SOURCE clangtoolastreader.cpp PROPERTY SKIP_AUTOMOC ON)
endif()
# special case end

#### Keys ignored in scope 6:.:.:lupdate.pro:NOT QMAKE_DEFAULT_LIBDIRS___contains____ss_CLANG_LIBDIR AND NOT disable_external_rpath:
# QMAKE_RPATHDIR = "$$CLANG_LIBDIR"

#### Keys ignored in scope 9:.:.:lupdate.pro:MINGW:
# RC_FILE = "lupdate.rc"
