
cmake_minimum_required(VERSION 2.8)

project(qmake_cmake_files)

macro(_do_build _dir)
    try_compile(Result ${CMAKE_CURRENT_BINARY_DIR}/${_dir}
        ${CMAKE_CURRENT_SOURCE_DIR}/${_dir}
        ${_dir}
        OUTPUT_VARIABLE Out
    )
endmacro()

macro(expect_pass _dir)
    _do_build(${_dir})
    if (NOT Result)
        message(SEND_ERROR "Build failed: ${Out}")
    endif()
endmacro()

macro(expect_fail _dir)
    _do_build(${_dir})
    if (Result)
        message(SEND_ERROR "Build should fail, but did not: ${Out}")
    endif()
endmacro()

expect_pass(pass1)
expect_pass(pass2)

