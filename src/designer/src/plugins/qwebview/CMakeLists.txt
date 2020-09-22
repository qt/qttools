# Generated from qwebview.pro.

#####################################################################
## QWebViewPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWebViewPlugin
    OUTPUT_NAME qwebview
    TYPE designer
    SOURCES
        qwebview_plugin.cpp qwebview_plugin.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Designer
        Qt::Gui
        Qt::WebKitWidgets
        Qt::Widgets
)

# Resources:
set(qwebview_plugin_resource_files
    "images/qwebview.png"
)

qt_internal_add_resource(QWebViewPlugin "qwebview_plugin"
    PREFIX
        "/qt-project.org/qwebview"
    FILES
        ${qwebview_plugin_resource_files}
)


#### Keys ignored in scope 1:.:.:qwebview.pro:<TRUE>:
# OTHER_FILES = "qwebview.json"
