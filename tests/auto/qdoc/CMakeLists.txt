# Generated from qdoc.pro.

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
