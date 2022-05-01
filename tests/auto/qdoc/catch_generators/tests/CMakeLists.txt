qt_internal_add_test(tst_QDoc_Catch_Generators
  SOURCES
    main.cpp
    catch_qchar_generator.cpp
    catch_qstring_generator.cpp
    catch_k_partition_of_r_generator.cpp
    catch_path_generator.cpp

    combinators/catch_oneof_generator.cpp
    combinators/catch_cycle_generator.cpp

    utilities/catch_generator_handler.cpp
  INCLUDE_DIRECTORIES
    ../src
    ../../catch
    ../../catch_conversions
)
