# Generated from makeqpf.pro.

#####################################################################
## makeqpf Binary:
#####################################################################

qt_add_executable(makeqpf
    SOURCES
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        qpf2.cpp qpf2.h
    DEFINES
        QT_DISABLE_DEPRECATED_BEFORE=0
        QT_NO_FREETYPE
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)

# Resources:
set(makeqpf_resource_files
    "Blocks.txt"
)

qt_add_resource(makeqpf "makeqpf"
    PREFIX
        "/"
    FILES
        ${makeqpf_resource_files}
)


#### Keys ignored in scope 1:.:.:makeqpf.pro:<TRUE>:
# QT_FOR_CONFIG = "tools-private"
# _REQUIREMENTS = "qtConfig(makeqpf)"
