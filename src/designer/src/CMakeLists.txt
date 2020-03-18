# Generated from src.pro.

add_subdirectory(uiplugin)
add_subdirectory(uitools)
if(QT_FEATURE_process)
    add_subdirectory(lib)
    add_subdirectory(components)
    add_subdirectory(designer)
endif()
if(QT_BUILD_SHARED_LIBS AND QT_FEATURE_process)
    add_subdirectory(plugins)
endif()
