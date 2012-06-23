
cmake_minimum_required(VERSION 2.8)

project(test_modules)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)

find_package(Qt5Designer REQUIRED)
find_package(Qt5Help REQUIRED)
find_package(Qt5UiTools REQUIRED)

include_directories(
  ${Qt5Designer_INCLUDE_DIRS}
  ${Qt5Help_INCLUDE_DIRS}
  ${Qt5UiTools_INCLUDE_DIRS}
)

add_definitions(
  ${Qt5Designer_DEFINITIONS}
  ${Qt5Help_DEFINITIONS}
  ${Qt5UiTools_DEFINITIONS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")


add_executable(mainapp main.cpp)
target_link_libraries(mainapp
  ${Qt5Designer_LIBRARIES}
  ${Qt5Help_LIBRARIES}
  ${Qt5UiTools_LIBRARIES}
)
