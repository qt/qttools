# Generated from examples.pro.

qt_examples_build_begin()

if(TARGET Qt::Widgets)
    add_subdirectory(help)
    add_subdirectory(linguist)
    add_subdirectory(uitools)
endif()
if(QT_FEATURE_process AND TARGET Qt::Widgets)
    add_subdirectory(designer)
endif()

qt_examples_build_end()
