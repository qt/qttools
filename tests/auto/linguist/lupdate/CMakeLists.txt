# Generated from lupdate.pro.

# special case begin
if(CMAKE_VERSION VERSION_LESS "3.19" AND MSVC AND QT_FEATURE_debug_and_release)
    message(WARNING "tst_lupdate will not be built in this configuration.")
    return()
endif()
# special case end

#####################################################################
## tst_lupdate Test:
#####################################################################

qt_internal_add_test(tst_lupdate
    SOURCES
        tst_lupdate.cpp
    LIBRARIES
        Qt::Tools
        Qt::CorePrivate
)

# The recursive tests take a bit longer for the clang-based parser.
# Increase the test timeout.
set_property(TEST tst_lupdate APPEND PROPERTY ENVIRONMENT "QTEST_FUNCTION_TIMEOUT=900000")
