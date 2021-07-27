# Generated from assistant.pro.

# special case begin
if(NOT TARGET Qt::Sql OR NOT TARGET Qt::PrintSupport)
    return()
endif()
# special case end

qt_exclude_tool_directories_from_default_target(
    assistant
    qhelpgenerator
)

if(NOT TARGET Qt::Sql)
    return()
endif()
if(NOT QT_FEATURE_assistant)
    return()
endif()
add_subdirectory(help)
add_subdirectory(assistant)
add_subdirectory(qhelpgenerator)

set(QLITEHTML_BIN_PATH ${INSTALL_BINDIR})
set(QLITEHTML_LIBRARY_PATH ${INSTALL_LIBDIR})
set(QLITEHTML_LIBRARY_TYPE STATIC)
set(BUILD_SHARED_LIBS OFF)
if(QT_FEATURE_static_runtime AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
add_subdirectory(qlitehtml/src EXCLUDE_FROM_ALL)
if(TARGET qlitehtml)
    qt_autogen_tools_initial_setup(qlitehtml)
    qt_internal_set_exceptions_flags(litehtml OFF)
    qt_disable_warnings(litehtml)
    qt_disable_warnings(qlitehtml)
    qt_disable_warnings(gumbo)
    qt_handle_multi_config_output_dirs(qlitehtml)
    set_target_properties(qlitehtml PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_BINDIR}"
        LIBRARY_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_LIBDIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_LIBDIR}")
else()
    message(FATAL_ERROR "qlitehtml not found. Did you run git submodule update --init --recursive?")
endif()
