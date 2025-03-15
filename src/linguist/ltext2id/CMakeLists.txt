# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## ltext2id Tool:
#####################################################################

qt_get_tool_target_name(target_name ltext2id)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt Migration Tool From Text-Based to Id-Based Translation"
    TOOLS_TARGET Linguist
    SOURCES
        main.cpp
        utils.h utils.cpp
        filetransformer.h filetransformer.cpp
        fileverifier.h fileverifier.cpp
        ../shared/parsers/trparser.cpp ../shared/parsers/trparser.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/parsers/xmlparser.cpp ../shared/parsers/xmlparser.h
        ../shared/numerus.cpp
        ../shared/ts.cpp
        ../shared/parsers/cpp.cpp ../shared/parsers/cpp.h
        ../shared/parsers/metastrings.h
        ../shared/parsers/metastrings.cpp
        ../shared/parsers/java.cpp
        ../shared/parsers/python.cpp
        ../shared/parsers/ui.cpp
        recorddirectory.h recorddirectory.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::Tools
)

qt_internal_return_unless_building_tools()

qt_internal_extend_target(${target_name} CONDITION TARGET Qt::QmlPrivate
    SOURCES
        ../shared/parsers/qdeclarative.cpp
    LIBRARIES
        Qt::QmlPrivate
)

qt_internal_extend_target(${target_name} CONDITION NOT TARGET Qt::QmlPrivate
    DEFINES
        QT_NO_QML
)
