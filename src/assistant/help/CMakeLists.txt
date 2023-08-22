# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Help Module:
#####################################################################

qt_internal_add_module(Help
    SOURCES
        qcompressedhelpinfo.cpp qcompressedhelpinfo.h
        qfilternamedialog.cpp qfilternamedialog.ui qfilternamedialog_p.h
        qhelp_global.cpp qhelp_global.h
        qhelpcollectionhandler.cpp qhelpcollectionhandler_p.h
        qhelpcontentwidget.cpp qhelpcontentwidget.h
        qhelpdbreader.cpp qhelpdbreader_p.h
        qhelpengine.cpp qhelpengine.h qhelpengine_p.h
        qhelpenginecore.cpp qhelpenginecore.h
        qhelpfilterdata.cpp qhelpfilterdata.h
        qhelpfilterengine.cpp qhelpfilterengine.h
        qhelpfiltersettings.cpp qhelpfiltersettings_p.h
        qhelpfiltersettingswidget.cpp qhelpfiltersettingswidget.h qhelpfiltersettingswidget.ui
        qhelpindexwidget.cpp qhelpindexwidget.h
        qhelplink.cpp qhelplink.h
        qhelpsearchengine.cpp qhelpsearchengine.h
        qhelpsearchindexreader.cpp qhelpsearchindexreader_p.h
        qhelpsearchindexreader_default.cpp qhelpsearchindexreader_default_p.h
        qhelpsearchindexwriter_default.cpp qhelpsearchindexwriter_default_p.h
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
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
    ENABLE_AUTOGEN_TOOLS
        uic
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

