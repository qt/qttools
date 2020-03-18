# Generated from qdbusviewer.pro.

#####################################################################
## qdbusviewer Binary:
#####################################################################

qt_add_executable(qdbusviewer
    GUI
    SOURCES
        logviewer.cpp logviewer.h
        main.cpp
        mainwindow.cpp mainwindow.h
        propertydialog.cpp propertydialog.h
        qdbusmodel.cpp qdbusmodel.h
        qdbusviewer.cpp qdbusviewer.h
        servicesproxymodel.cpp servicesproxymodel.h
    PUBLIC_LIBRARIES
        Qt::DBusPrivate
        Qt::Gui
        Qt::Widgets
        Qt::Xml
)

# Resources:
set(qdbusviewer_resource_files
    "images/qdbusviewer-128.png"
    "images/qdbusviewer.png"
)

qt_add_resource(qdbusviewer "qdbusviewer"
    PREFIX
        "/qt-project.org/qdbusviewer"
    FILES
        ${qdbusviewer_resource_files}
)


## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:qdbusviewer.pro:APPLE:
# ICON = "images/qdbusviewer.icns"
# QMAKE_INFO_PLIST = "Info_mac.plist"

#### Keys ignored in scope 3:.:.:qdbusviewer.pro:WIN32:
# RC_FILE = "qdbusviewer.rc"
