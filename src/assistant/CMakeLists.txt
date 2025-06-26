# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_FEATURE_fullqthelp)
    add_subdirectory(help)
    add_subdirectory(plugins)
    return()
endif()

if(NOT TARGET Qt::Sql OR NOT TARGET Qt::PrintSupport)
    return()
endif()

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
add_subdirectory(plugins)

set(QLITEHTML_BIN_PATH ${INSTALL_BINDIR})
set(QLITEHTML_LIBRARY_PATH ${INSTALL_LIBDIR})
set(QLITEHTML_LIBRARY_TYPE STATIC)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
if(QT_FEATURE_static_runtime AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
add_subdirectory(qlitehtml/src EXCLUDE_FROM_ALL)
if(TARGET qlitehtml)
    qt_autogen_tools_initial_setup(qlitehtml)
    # The litehtml and gumbo targets will not be available here if they are not built by Qt
    # but found in the system, because they are imported only to the subdirectory scope
    # where find_package was called. But that's fine, we wouldn't be able to set compiler flags
    # on them anyway.
    if(TARGET litehtml)
        qt_internal_set_exceptions_flags(litehtml OFF)
        qt_disable_warnings(litehtml)
    endif()
    if(TARGET gumbo)
        qt_disable_warnings(gumbo)
    endif()
    qt_disable_warnings(qlitehtml)
    target_link_libraries(qlitehtml PRIVATE Qt::PlatformCommonInternal)
    set_target_properties(qlitehtml PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_BINDIR}"
        LIBRARY_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_LIBDIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_LIBDIR}")
    qt_handle_multi_config_output_dirs(qlitehtml)
else()
    message(FATAL_ERROR "qlitehtml not found. Did you run git submodule update --init --recursive?")
endif()
