# Generated from qdoc.pro.

# special case begin
if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND QT_FEATURE_debug_and_release)
    message(WARNING "qdoc tests will not be built in this configuration.")
    return()
endif()
# special case end

add_subdirectory(config)
# special case begin
# CMake builds currently don't build qdoc, presumably because the llvm
# version update hasn't been finished yet. This causes the test to fail.
# Include it conditionally.
if(TARGET Qt::qdoc)
    add_subdirectory(generatedoutput)
endif()
# special case end
add_subdirectory(qdoccommandlineparser)
add_subdirectory(utilities)
