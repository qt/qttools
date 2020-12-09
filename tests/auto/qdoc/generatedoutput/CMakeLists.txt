# Generated from generatedoutput.pro.
# special case skip regeneration
# It's mostly manually written.

#####################################################################
## tst_generatedOutput Test:
#####################################################################

qt_internal_add_test(tst_generatedOutput
    SOURCES
        tst_generatedoutput.cpp
)

# Write relevant Qt include path to a file, to be read in by QDoc
set(framework_path "\n")
set(includepathsfile "${CMAKE_CURRENT_BINARY_DIR}/qdocincludepaths.inc")
find_package(Qt6 COMPONENTS Core REQUIRED)
if(Qt6Core_FOUND)
    get_target_property(include_paths Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
endif()

while(include_paths)
    list(POP_BACK include_paths inc_path)
    if(inc_path MATCHES "(.+)/QtCore\.framework$")
        string(APPEND framework_path "-F${CMAKE_MATCH_1}")
        break()
    endif()
endwhile()

set (include_paths "$<TARGET_PROPERTY:tst_generatedOutput,INCLUDE_DIRECTORIES>")
file(GENERATE OUTPUT ${includepathsfile} CONTENT "-I$<JOIN:${include_paths},\n-I>${framework_path}")
