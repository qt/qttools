
# This is an automatic test for the CMake configuration files.
# To run it,
# 1) mkdir build   # Create a build directory
# 2) cd build
# 3) cmake ..      # Run cmake on this directory.
# 4) ctest         # Run ctest
#
# The expected output is something like:
#
#     Start 1: pass1
# 1/2 Test #1: pass1 ............................   Passed    1.88 sec
#     Start 2: pass2
# 2/2 Test #2: pass2 ............................   Passed    0.60 sec
#
# Note that if Qt is not installed, or if it is installed to a
# non-standard prefix, the environment variable CMAKE_PREFIX_PATH
# needs to be set to the installation prefix or build prefix of Qt
# before running these tests.

cmake_minimum_required(VERSION 2.8)

project(qmake_cmake_files)

enable_testing()

macro(expect_pass _dir)
  string(REPLACE "(" "_" testname "${_dir}")
  string(REPLACE ")" "_" testname "${testname}")
  add_test(${testname} ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}"
    "${CMAKE_CURRENT_BINARY_DIR}/${_dir}"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
  )
endmacro()

macro(expect_fail _dir)
  string(REPLACE "(" "_" testname "${_dir}")
  string(REPLACE ")" "_" testname "${testname}")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/failbuild/${_dir}")
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/failbuild/${_dir}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/failbuild/${_dir}/CMakeLists.txt"
    "
      cmake_minimum_required(VERSION 2.8)
      project(${_dir}_build)

      try_compile(Result \${CMAKE_CURRENT_BINARY_DIR}/${_dir}
          \${CMAKE_CURRENT_SOURCE_DIR}/${_dir}
          ${_dir}
          OUTPUT_VARIABLE Out
      )
      message(\"\${Out}\")
      if (Result)
        message(SEND_ERROR \"Succeeded build which should fail\")
      endif()
      "
  )
  add_test(${testname} ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMAKE_CURRENT_BINARY_DIR}/failbuild/${_dir}"
    "${CMAKE_CURRENT_BINARY_DIR}/failbuild/${_dir}/build"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
  )
endmacro()

expect_pass(pass1)
expect_pass(pass2)

