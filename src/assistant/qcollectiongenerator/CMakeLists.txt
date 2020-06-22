# Generated from qcollectiongenerator.pro.

#####################################################################
## qcollectiongenerator Tool:
#####################################################################

qt_get_tool_target_name(target_name qcollectiongenerator)
qt_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Help Collection File Generator"
    TOOLS_TARGET Tools # special case
    SOURCES
        main.c
    PUBLIC_LIBRARIES
        Qt::Gui
)

#### Keys ignored in scope 1:.:.:qcollectiongenerator.pro:<TRUE>:
# QMAKE_TARGET_DESCRIPTION = "Qt Help Collection File Generator"
