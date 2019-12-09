# Generated from plugins.pro.

if(TARGET Qt::WebKitWidgets)
    add_subdirectory(qwebview)
endif()
if(TARGET Qt::AxContainer AND WIN32)
    add_subdirectory(activeqt)
endif()
if(TARGET Qt::QuickWidgets)
    add_subdirectory(qquickwidget)
endif()
