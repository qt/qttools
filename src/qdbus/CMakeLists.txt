# Generated from qdbus.pro.

if(QT_FEATURE_dom)
    add_subdirectory(qdbus)
endif()
if(QT_FEATURE_dialogbuttonbox AND QT_FEATURE_inputdialog AND QT_FEATURE_menu AND QT_FEATURE_messagebox AND TARGET Qt::Widgets)
    add_subdirectory(qdbusviewer)
endif()
