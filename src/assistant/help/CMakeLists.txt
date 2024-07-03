# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Help Module:
#####################################################################

qt_internal_add_module(Help
    PLUGIN_TYPES help
    SOURCES
    # QtHelpCore
        qcompressedhelpinfo.cpp qcompressedhelpinfo.h
        qhelp_global.cpp qhelp_global.h
        qhelpcollectionhandler.cpp qhelpcollectionhandler_p.h
        qhelpcontentitem.cpp qhelpcontentitem.h
        qhelpdbreader.cpp qhelpdbreader_p.h
        qhelpenginecore.cpp qhelpenginecore.h
        qhelpfilterdata.cpp qhelpfilterdata.h
        qhelpfilterengine.cpp qhelpfilterengine.h
        qhelplink.cpp qhelplink.h
        qhelpsearchenginecore.cpp qhelpsearchenginecore.h
        qhelpsearchindexreader.cpp qhelpsearchindexreader_p.h
        qhelpsearchindexwriter.cpp qhelpsearchindexwriter_p.h
        qhelpsearchresult.cpp qhelpsearchresult.h
    # QtHelp
        qfilternamedialog.cpp qfilternamedialog.ui qfilternamedialog_p.h
        qhelpcontentwidget.cpp qhelpcontentwidget.h
        qhelpengine.cpp qhelpengine.h
        qhelpfiltersettingswidget.cpp qhelpfiltersettingswidget.h qhelpfiltersettingswidget.ui
        qhelpindexwidget.cpp qhelpindexwidget.h
        qhelpsearchengine.cpp qhelpsearchengine.h
        qhelpsearchquerywidget.cpp qhelpsearchquerywidget.h
        qhelpsearchresultwidget.cpp qhelpsearchresultwidget.h
        qoptionswidget.cpp qoptionswidget_p.h
    DEFINES
        # -QT_ASCII_CAST_WARNINGS # special case remove
        QHELP_LIB
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Sql
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
    NO_GENERATE_CPP_EXPORTS
)

# Resources:
set(helpsystem_resource_files
    "images/1leftarrow.png"
    "images/1rightarrow.png"
    "images/3leftarrow.png"
    "images/3rightarrow.png"
    "images/mac/minus.png"
    "images/mac/plus.png"
    "images/win/minus.png"
    "images/win/plus.png"
)

qt_internal_add_resource(Help "helpsystem"
    PREFIX
        "/qt-project.org/assistant"
    FILES
        ${helpsystem_resource_files}
)

qt_internal_add_docs(Help
    doc/qthelp.qdocconf
)

