# Generated from designer.pro.


qt_exclude_tool_directories_from_default_target(
    containerextension
    taskmenuextension
)

add_subdirectory(calculatorform)
if(QT_BUILD_SHARED_LIBS AND NOT solaris-cc_x_)
    add_subdirectory(calculatorbuilder)
    add_subdirectory(worldtimeclockbuilder)
endif()
if(QT_BUILD_SHARED_LIBS)
    add_subdirectory(containerextension)
    add_subdirectory(customwidgetplugin)
    add_subdirectory(taskmenuextension)
    add_subdirectory(worldtimeclockplugin)
endif()
