qt_internal_add_module(QDocCatchConversionsPrivate
    INTERNAL_MODULE
    HEADER_MODULE
    EXTERNAL_HEADERS_DIR src
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_extend_target(QDocCatchConversionsPrivate
    PUBLIC_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QtQDocCatchConversions>
)
