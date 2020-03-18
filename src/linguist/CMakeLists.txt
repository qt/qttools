# Generated from linguist.pro.

add_subdirectory(lconvert)
add_subdirectory(lprodump)
add_subdirectory(lrelease)
add_subdirectory(lrelease-pro)
add_subdirectory(lupdate)
add_subdirectory(lupdate-pro)
if(QT_FEATURE_process AND QT_FEATURE_pushbutton AND QT_FEATURE_toolbutton AND TARGET Qt::Widgets AND NOT no-png)
    add_subdirectory(linguist)
endif()

# special case begin
# Create a fake module that would emulate the Qt5::LinguistTools CMake Config package
add_qt_module(Linguist NO_MODULE_HEADERS HEADER_MODULE)
# special case end
