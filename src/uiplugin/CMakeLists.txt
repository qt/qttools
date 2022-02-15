# Generated from uiplugin.pro.

#####################################################################
## UiPlugin Module:
#####################################################################

qt_internal_add_module(UiPlugin
    NO_PRIVATE_MODULE
    HEADER_MODULE
    QMAKE_MODULE_CONFIG designer_defines
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

# special case begin
set(is_plugin "$<TARGET_PROPERTY:QT_PLUGIN_CLASS_NAME>")
target_compile_definitions(
    UiPlugin
    INTERFACE
        $<$<BOOL:${is_plugin}>:QDESIGNER_EXPORT_WIDGETS>
)
# special case end

#### Keys ignored in scope 1:.:.:uiplugin.pro:<TRUE>:
# MODULE_CONFIG = "designer_defines"
