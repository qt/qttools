# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtTools
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Tools"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# add platform specific compontents
set(optional_components "")
if(WIN32)
    list(APPEND optional_components AxContainer)
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals)

# This should be called as early as possible, just after find_package(BuildInternals) where it is
# defined.
qt_internal_project_setup()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Core)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS
             DBus InputSupportPrivate Network Xml Widgets Quick QuickWidgets Qml QmlLSPrivate
             Sql PrintSupport OpenGL OpenGLWidgets QuickLayouts ${optional_components})

qt_build_repo()

# Add tool dependencies that were deferred by qt_internal_add_docs.
qt_internal_add_deferred_dependencies()
