qt_internal_add_test(tst_QDoc
  SOURCES
    main.cpp

    boundaries/filesystem/catch_filepath.cpp
    boundaries/filesystem/catch_directorypath.cpp
    filesystem/catch_fileresolver.cpp

    ../../../../src/qdoc/boundaries/filesystem/filepath.cpp
    ../../../../src/qdoc/boundaries/filesystem/directorypath.cpp
    ../../../../src/qdoc/boundaries/filesystem/resolvedfile.cpp
    ../../../../src/qdoc/filesystem/fileresolver.cpp
  INCLUDE_DIRECTORIES
    ../../../../src/qdoc/
    ../catch/
    ../catch_generators/
    ../catch_conversions/
)
