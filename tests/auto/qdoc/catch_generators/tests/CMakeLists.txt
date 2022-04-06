qt_internal_add_test(tst_QDoc_Catch_Generators
  SOURCES
    main.cpp

    catch_qchar_generator.cpp
    catch_qstring_generator.cpp
  INCLUDE_DIRECTORIES
    ../src
    ../../catch
    ../../catch_conversions
)
