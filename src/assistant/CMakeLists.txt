# Generated from assistant.pro.

# special case begin
if(NOT TARGET Qt::Sql OR NOT TARGET Qt::PrintSupport)
    return()
endif()
# special case end

qt_exclude_tool_directories_from_default_target(
    qhelpgenerator
)

if(NOT TARGET Qt::Sql)
    return()
endif()
if(NOT QT_FEATURE_help)
    return()
endif()
add_subdirectory(help)
add_subdirectory(qhelpgenerator)
