
cmake_minimum_required(VERSION 2.8)

project(pass2)

find_package(Qt5Core REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
)

set(my_srcs myi18nobject.cpp)

qt5_create_translation(qm_files ${my_srcs} myobject_de.ts)

add_executable(myobject ${my_srcs} ${qm_files})
target_link_libraries(myobject ${Qt5Core_LIBRARIES})
