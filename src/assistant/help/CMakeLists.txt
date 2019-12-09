# Generated from help.pro.

#####################################################################
## Help Module:
#####################################################################

qt_add_module(Help
    SOURCES
        qcompressedhelpinfo.cpp qcompressedhelpinfo.h
        qhelp_global.cpp qhelp_global.h
        qhelpcollectionhandler.cpp qhelpcollectionhandler_p.h
        qhelpcontentwidget.cpp qhelpcontentwidget.h
        qhelpdbreader.cpp qhelpdbreader_p.h
        qhelpengine.cpp qhelpengine.h qhelpengine_p.h
        qhelpenginecore.cpp qhelpenginecore.h
        qhelpfilterdata.cpp qhelpfilterdata.h
        qhelpfilterengine.cpp qhelpfilterengine.h
        qhelpindexwidget.cpp qhelpindexwidget.h
        qhelpsearchengine.cpp qhelpsearchengine.h
        qhelpsearchindexreader.cpp qhelpsearchindexreader_p.h
        qhelpsearchindexreader_default.cpp qhelpsearchindexreader_default_p.h
        qhelpsearchindexwriter_default.cpp qhelpsearchindexwriter_default_p.h
        qhelpsearchquerywidget.cpp qhelpsearchquerywidget.h
        qhelpsearchresultwidget.cpp qhelpsearchresultwidget.h
    DEFINES
        # -QT_ASCII_CAST_WARNINGS # special case remove
        QHELP_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Sql
        Qt::Widgets
)

# Resources:
set(helpsystem_resource_files
    "images/1leftarrow.png"
    "images/1rightarrow.png"
    "images/3leftarrow.png"
    "images/3rightarrow.png"
)

qt_add_resource(Help "helpsystem"
    PREFIX
        "/qt-project.org/assistant"
    FILES
        ${helpsystem_resource_files}
)

qt_add_docs(Help
    doc/qthelp.qdocconf
)

