qt_internal_add_3rdparty_header_module(QDocCatchPrivate
    EXTERNAL_HEADERS_DIR include
)

qt_internal_extend_target(QDocCatchPrivate
    PUBLIC_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QtQDocCatch>
)
