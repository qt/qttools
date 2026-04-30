# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Check LLVM_ZSTD_MARKER for reasoning
find_package(WrapZSTD QUIET)
set(__qt_wrapllvm_CMAKE_DISABLE_FIND_PACKAGE_zstd ${CMAKE_DISABLE_FIND_PACKAGE_zstd})
if(WrapZSTD_FOUND)
    set(CMAKE_DISABLE_FIND_PACKAGE_zstd TRUE)
endif()

qt_find_package(LLVM CONFIG REQUIRED)

set(CMAKE_DISABLE_FIND_PACKAGE_zstd ${__qt_wrapllvm_CMAKE_DISABLE_FIND_PACKAGE_zstd})

llvm_map_components_to_libnames(llvm_libs Support Object)

qt_get_tool_target_name(target_name objcnamemangler)

qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Objective-C Name Mangler"
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    TOOLS_TARGET Tools
    SOURCES
        objcnamemangler.cpp
    LIBRARIES
        ${llvm_libs}
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}ObjCNameManglerMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}ObjCNameManglerMacros.cmake"
)

qt_internal_return_unless_building_tools()

target_include_directories(${target_name} SYSTEM PRIVATE "${LLVM_INCLUDE_DIRS}")
