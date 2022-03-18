qt_examples_build_begin(EXTERNAL_BUILD)

qt_exclude_tool_directories_from_default_target(
    assistant
)

if(TARGET Qt::Widgets)
    if(TARGET Qt::Help)
        add_subdirectory(help)
    endif()
    add_subdirectory(linguist)
    add_subdirectory(uitools)
endif()
if(QT_FEATURE_process AND TARGET Qt::Widgets)
    add_subdirectory(designer)
    add_subdirectory(assistant)
endif()

qt_examples_build_end()
