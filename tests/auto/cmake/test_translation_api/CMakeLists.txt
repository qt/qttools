cmake_minimum_required(VERSION 3.15)

project(test_update_translation_macro)

find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)

function(create_app target)
    add_executable(${target}
        myi18nobject.cpp)

    target_include_directories(${target}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/some_dir)

    target_link_libraries(${target} Qt6::Core)
endfunction()

function(expect_target target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Expected target '${target}' does not exist.")
    endif()
endfunction()

function(not_expect_target target)
    if(TARGET ${target})
        message(FATAL_ERROR "Target '${target}' exists unexpectedly.")
    endif()
endfunction()

# Test NO_GLOBAL_TARGET for lupdate.
create_app(app1)
qt6_add_lupdate(app1
    NO_GLOBAL_TARGET
    TS_FILES myobject_de.ts)
expect_target(app1_lupdate)
not_expect_target(app1_lrelease)
not_expect_target(update_translations)
not_expect_target(release_translations)

# Test NO_GLOBAL_TARGET for lrelease.
create_app(app2)
qt6_add_lrelease(app2
    NO_GLOBAL_TARGET
    TS_FILES myobject_de.ts myobject_en.ts)
not_expect_target(app2_lupdate)
expect_target(app2_lrelease)
not_expect_target(update_translations)
not_expect_target(release_translations)

# Typical usage of qt_add_lupdate/qt_add_lrelease. Pass some options for good measure.
create_app(app3)
qt6_add_lupdate(app3
    TS_FILES myobject_no.ts myobject_fi.ts
    OPTIONS -source-language en_US)
qt6_add_lrelease(app3
    TS_FILES myobject_no.ts myobject_fi.ts
    OPTIONS -compress)
expect_target(app3_lupdate)
expect_target(app3_lrelease)
expect_target(release_translations)

# Now do the same with qt6_add_translations.
create_app(app4)
qt6_add_translations(app4
    TS_FILES myobject_no.ts myobject_fi.ts
    LUPDATE_OPTIONS -source-language en_US
    LRELEASE_OPTIONS -compress)
expect_target(app4_lupdate)
expect_target(app4_lrelease)
expect_target(release_translations)

# Build the update_translations target
add_custom_target(my_all_target
    ALL
    DEPENDS update_translations)
