# Generated from qttools.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtTools # special case
    VERSION 6.0.0
    DESCRIPTION "Qt Tools" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core) # special case
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Xml Widgets Quick QuickWidgets) # special case

qt_build_repo()

