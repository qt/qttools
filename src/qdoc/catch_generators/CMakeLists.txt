qt_internal_add_module(QDocCatchGeneratorsPrivate
    HEADER_MODULE
    EXTERNAL_HEADERS_DIR src
)

qt_internal_extend_target(QDocCatchGeneratorsPrivate
    PUBLIC_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QtQDocCatchGeneratorsPrivate>
)

if(QT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
