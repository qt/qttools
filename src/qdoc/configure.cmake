

#### Inputs



#### Libraries



#### Tests

# libclang
qt_find_package(WrapLibClang PROVIDED_TARGETS WrapLibClang::WrapLibClang)

if(TARGET WrapLibClang::WrapLibClang)
    set(TEST_libclang "ON" CACHE BOOL "Required libclang version found." FORCE)
endif()



#### Features

qt_feature("qdoc" PRIVATE
    LABEL "QDoc"
    CONDITION TEST_libclang
)
