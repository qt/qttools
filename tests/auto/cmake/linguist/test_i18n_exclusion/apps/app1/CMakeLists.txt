# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_executable(app1
    main.cpp
    excluded1.cpp
    subdir/excluded2.cpp
    subdir/subdir3/excluded5.cpp
    ../shared/excluded3.cpp
)

# Note: subdir2/excluded4.cpp is deliberately not added to the sources. It's included in main.cpp,
# and lupdate picks up such source files. The exclusion pattern must work for those too.

set_property(TARGET app1 PROPERTY QT_EXCLUDE_SOURCES_FROM_TRANSLATION
    ../shared/*
    excluded?.cpp
    subdir/*
    subdir2/excluded4.cpp
)
target_link_libraries(app1 PRIVATE
    Qt6::Gui
    lib1
    lib2
)
