# Generated from qttools.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtTools # special case
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Tools" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# special case begin
set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)

# Make sure we only use latest private CMake API, aka no compatibility wrappers.
set(QT_NO_INTERNAL_COMPATIBILITY_FUNCTIONS TRUE)

# Make sure we use the fixed BASE argument of qt_add_resource.
set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)

# add platform specific compontents
set(optional_components "")
if(WIN32)
    list(APPEND optional_components AxContainer)
endif()

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} QUIET CONFIG OPTIONAL_COMPONENTS
             DBus Xml Widgets Quick QuickWidgets QmlDevToolsPrivate
             Sql PrintSupport OpenGL OpenGLWidgets ${optional_components})
# special case end

qt_build_repo()

# special case begin

# Add tool dependencies that were deferred by qt_internal_add_docs.
qt_internal_add_deferred_dependencies()

# special case end
