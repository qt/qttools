# Generated from assistant.pro.

if(NOT TARGET Qt::Sql OR NOT TARGET Qt::PrintSupport) # special case
    return()
endif()
add_subdirectory(help)
add_subdirectory(assistant)
add_subdirectory(qhelpgenerator)
add_subdirectory(qcollectiongenerator)
