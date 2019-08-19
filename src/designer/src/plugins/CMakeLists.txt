# Generated from plugins.pro.

if(TARGET webkitwidgets)
    add_subdirectory(qwebview)
endif()
if(TARGET axcontainer AND WIN32)
    add_subdirectory(activeqt)
endif()
if(TARGET Qt::QuickWidgets)
    add_subdirectory(qquickwidget)
endif()
