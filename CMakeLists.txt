# Generated from qttools.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtTools # special case
    VERSION 6.0.0
    DESCRIPTION "Qt Tools" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# special case begin
set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS AxContainer DBus Xml Widgets Quick QuickWidgets QmlDevTools Sql PrintSupport OpenGL OpenGLWidgets)
# special case end

qt_build_repo()

