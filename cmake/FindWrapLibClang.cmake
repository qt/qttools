if(TARGET WrapLibClang::WrapLibClang)
  set(WrapLibClang_FOUND TRUE)
  return()
endif()

find_package(Clang CONFIG)

if(TARGET libclang AND TARGET clangHandleCXX)
  get_target_property(type libclang TYPE)
  if (MSVC AND type STREQUAL "STATIC_LIBRARY")
    if (NOT CMAKE_BUILD_TYPE MATCHES "(Release|MinSizeRel|RelWithDebInfo)")
      message(STATUS "Static linkage against libclang with MSVC was requested, but the build is not a release build, therefore libclang cannot be used.")
      set(WrapLibClang_FOUND FALSE)
      return()
    endif()
  endif()

  add_library(WrapLibClang::WrapLibClang IMPORTED INTERFACE)

  target_include_directories(WrapLibClang::WrapLibClang INTERFACE ${CLANG_INCLUDE_DIRS})
  if (NOT TARGET Threads::Threads)
    find_package(Threads)
  endif()
  target_link_libraries(WrapLibClang::WrapLibClang INTERFACE libclang clangHandleCXX Threads::Threads)

  foreach(version MAJOR MINOR PATCH)
    set(QT_LIB_CLANG_VERSION_${version} ${LLVM_VERSION_${version}} CACHE STRING "" FORCE)
  endforeach()
  set(QT_LIB_CLANG_VERSION ${LLVM_PACKAGE_VERSION} CACHE STRING "" FORCE)
  set(QT_LIB_CLANG_LIBDIR "${LLVM_LIBRARY_DIRS}" CACHE STRING "" FORCE)
  set(QT_LIBCLANG_RESOURCE_DIR "\"${QT_LIB_CLANG_LIBDIR}/clang/${QT_LIB_CLANG_VERSION}/include\"" CACHE STRING "" FORCE)

  set(WrapLibClang_FOUND TRUE)
endif()
