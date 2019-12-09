

#### Inputs



#### Libraries



#### Tests

# libclang
qt_find_package(WrapLibClang PROVIDED_TARGETS WrapLibClang::WrapLibClang)

if(TARGET WrapLibClang::WrapLibClang)
    set(TEST_libclang "ON" CACHE BOOL "Required libclang version found." FORCE)
endif()



#### Features

qt_feature("clang" PRIVATE
    LABEL "QDoc"
    CONDITION TEST_libclang
)
qt_feature("clangcpp" PRIVATE
    LABEL "Clang-based lupdate parser"
    CONDITION QT_FEATURE_clang AND TEST_libclang
)
