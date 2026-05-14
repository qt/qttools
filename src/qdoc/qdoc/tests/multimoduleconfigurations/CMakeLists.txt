# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_multiModuleConfigurations Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_multiModuleConfigurations LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# The fixtures here exercise TemplateHTML output and depend on the template
# generator. Gate on the same feature flag as the sibling test.
if(NOT QT_QDOC_ENABLE_TEMPLATE_GENERATOR)
    return()
endif()

qt_internal_add_test(tst_multiModuleConfigurations
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/tst_multimoduleconfigurations.cpp
)

# Write relevant Qt include path to a file, to be read in by QDoc
set(config_subfolder "")
get_cmake_property(is_multi_config GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
    set(config_subfolder "$<CONFIG>/")
endif()
set(includepathsfile "${CMAKE_CURRENT_BINARY_DIR}/${config_subfolder}qdocincludepaths.inc")
set(framework_path "\n")

find_package(Qt6 COMPONENTS Core REQUIRED)
if(Qt6Core_FOUND)
    get_target_property(include_paths Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
endif()

while(include_paths)
    list(POP_BACK include_paths inc_path)
    if(inc_path MATCHES "(.+)/QtCore\.framework$")
        string(APPEND framework_path "-F${CMAKE_MATCH_1}")
        break()
    endif()
endwhile()

set (include_paths "$<TARGET_PROPERTY:tst_multiModuleConfigurations,INCLUDE_DIRECTORIES>")
file(GENERATE OUTPUT ${includepathsfile} CONTENT "-I$<JOIN:${include_paths},\n-I>${framework_path}")
target_compile_definitions(tst_multiModuleConfigurations PRIVATE DOCINCPATH="${includepathsfile}")
add_dependencies(tst_multiModuleConfigurations Qt::qdoc)

