# Generated from qttools.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtTools # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Tools" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# add platform specific compontents
set(optional_components "")
if(WIN32)
    list(APPEND optional_components AxContainer)
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS
             DBus Xml Widgets Quick QuickWidgets Qml
             Sql PrintSupport OpenGL OpenGLWidgets ${optional_components})

qt_build_repo()

# special case begin

# Add tool dependencies that were deferred by qt_internal_add_docs.
qt_internal_add_deferred_dependencies()

# special case end
