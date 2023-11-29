qt_internal_add_module(QDocCatchGeneratorsPrivate
    INTERNAL_MODULE
    HEADER_MODULE
    EXTERNAL_HEADERS_DIR src
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_extend_target(QDocCatchGeneratorsPrivate
    PUBLIC_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/QtQDocCatchGenerators>
)

if(QT_BUILD_TESTS)
    add_subdirectory(tests)
endif()
