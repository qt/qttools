# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_QDoc_Catch_Generators
  SOURCES
    main.cpp
    generators/catch_qchar_generator.cpp
    generators/catch_qstring_generator.cpp
    generators/catch_k_partition_of_r_generator.cpp
    generators/catch_path_generator.cpp

    generators/combinators/catch_oneof_generator.cpp
    generators/combinators/catch_cycle_generator.cpp

    utilities/semantics/catch_generator_handler.cpp
  INCLUDE_DIRECTORIES
    ../src
    ../../catch
    ../../catch_conversions
)
