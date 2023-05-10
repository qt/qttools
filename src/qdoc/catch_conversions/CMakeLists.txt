qt_internal_add_module(QDocCatchConversionsPrivate
    HEADER_MODULE
    EXTERNAL_HEADERS_DIR src
)

qt_internal_extend_target(QDocCatchConversionsPrivate
    PUBLIC_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QtQDocCatchConversionsPrivate>
)
