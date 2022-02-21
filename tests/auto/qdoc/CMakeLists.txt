# Generated from qdoc.pro.

# special case begin
if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND QT_FEATURE_debug_and_release)
    message(WARNING "qdoc tests will not be built in this configuration.")
    return()
endif()
# special case end

add_subdirectory(catch_generators)
add_subdirectory(config)
add_subdirectory(generatedoutput)
add_subdirectory(qdoccommandlineparser)
add_subdirectory(utilities)
