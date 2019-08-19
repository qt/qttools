cmake_minimum_required(VERSION 3.15.0)

project(QtTools
    VERSION 6.0.0
    DESCRIPTION "Qt Tools"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core
                                           OPTIONAL_COMPONENTS Xml Widgets Quick QuickWidgets)
qt_build_repo()
