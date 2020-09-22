# Generated from qdoccommandlineparser.pro.

#####################################################################
## tst_qdoccommandlineparser Test:
#####################################################################

qt_internal_add_test(tst_qdoccommandlineparser
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" # special case
    SOURCES
        ../../../../src/qdoc/qdoccommandlineparser.cpp ../../../../src/qdoc/qdoccommandlineparser.h
        tst_qdoccommandlineparser.cpp
    INCLUDE_DIRECTORIES
        ../../../../src/qdoc
)

# Resources:
set(tst_qdoccommandlineparser_resource_files
    "tst_arguments.txt"
)

qt_internal_add_resource(tst_qdoccommandlineparser "tst_qdoccommandlineparser"
    PREFIX
        "/"
    FILES
        ${tst_qdoccommandlineparser_resource_files}
)

