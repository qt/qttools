# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_tool_target_name(target_name objcnamemangler)

qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Objective-C Name Mangler"
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    TOOLS_TARGET Tools
    SOURCES
        objcnamemangler.cpp
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}ObjCNameManglerMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}ObjCNameManglerMacros.cmake"
)

qt_internal_return_unless_building_tools()

# Check LLVM_ZSTD_MARKER for reasoning
find_package(WrapZSTD QUIET)
set(__qt_wrapllvm_CMAKE_DISABLE_FIND_PACKAGE_zstd ${CMAKE_DISABLE_FIND_PACKAGE_zstd})
if(WrapZSTD_FOUND)
    set(CMAKE_DISABLE_FIND_PACKAGE_zstd TRUE)
endif()

# The extra find_package call is needed here to make LLVM_INCLUDE_DIRS available, because
# none of the imported targets propagate that automatically.
qt_find_package(LLVM CONFIG REQUIRED)

set(CMAKE_DISABLE_FIND_PACKAGE_zstd ${__qt_wrapllvm_CMAKE_DISABLE_FIND_PACKAGE_zstd})

llvm_map_components_to_libnames(llvm_libs Support Object)
target_link_libraries(${target_name} PRIVATE ${llvm_libs})
target_include_directories(${target_name} SYSTEM PRIVATE "${LLVM_INCLUDE_DIRS}")
