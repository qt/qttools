# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_QDoc_Catch_Generators
  SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/generators/catch_qchar_generator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/generators/catch_qstring_generator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/generators/catch_k_partition_of_r_generator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/generators/catch_path_generator.cpp

    ${CMAKE_CURRENT_LIST_DIR}/generators/combinators/catch_oneof_generator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/generators/combinators/catch_cycle_generator.cpp

    ${CMAKE_CURRENT_LIST_DIR}/utilities/semantics/catch_generator_handler.cpp
  LIBRARIES
    Qt::QDocCatchPrivate
    Qt::QDocCatchConversionsPrivate
    Qt::QDocCatchGeneratorsPrivate
)
