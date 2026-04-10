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
    DEFINES
        # -QT_ASCII_CAST_WARNINGS # special case remove
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
        QHELP_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::Tools
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Sql
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_extend_target(Help CONDITION QT_FEATURE_concurrent
    LIBRARIES
        Qt::Concurrent
)

if(QT_FEATURE_fullqthelp)
    qt_internal_extend_target(Help
        SOURCES
            qfilternamedialog.cpp qfilternamedialog_p.h
            qhelpcontentwidget.cpp qhelpcontentwidget.h
            qhelpengine.cpp qhelpengine.h
            qhelpfiltersettingswidget.cpp qhelpfiltersettingswidget.h
            qhelpindexwidget.cpp qhelpindexwidget.h
            qhelpsearchengine.cpp qhelpsearchengine.h
            qhelpsearchquerywidget.cpp qhelpsearchquerywidget.h
            qhelpsearchresultwidget.cpp qhelpsearchresultwidget.h
            qoptionswidget.cpp qoptionswidget_p.h
        PUBLIC_LIBRARIES
            Qt::Gui
            Qt::Widgets
    )

    qt_add_ui(Help
        SOURCES
            qfilternamedialog.ui
            qhelpfiltersettingswidget.ui
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
endif()

qt_internal_add_docs(Help
    doc/qthelp.qdocconf
)

