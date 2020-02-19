TARGET = QtHelp

QT = core-private gui widgets sql
QT_PRIVATE = network

DEFINES += QHELP_LIB

QMAKE_DOCS = $$PWD/doc/qthelp.qdocconf

DEFINES -= QT_ASCII_CAST_WARNINGS

RESOURCES += helpsystem.qrc
SOURCES += \
    qcompressedhelpinfo.cpp \
    qfilternamedialog.cpp \
    qhelpenginecore.cpp \
    qhelpengine.cpp \
    qhelpfilterdata.cpp \
    qhelpfilterengine.cpp \
    qhelpfiltersettings.cpp \
    qhelpfiltersettingswidget.cpp \
    qhelpdbreader.cpp \
    qhelpcontentwidget.cpp \
    qhelpindexwidget.cpp \
    qhelplink.cpp \
    qhelpcollectionhandler.cpp \
    qhelpsearchengine.cpp \
    qhelpsearchquerywidget.cpp \
    qhelpsearchresultwidget.cpp \
    qhelpsearchindexwriter_default.cpp \
    qhelpsearchindexreader_default.cpp \
    qhelpsearchindexreader.cpp \
    qhelp_global.cpp \
    qoptionswidget.cpp

HEADERS += \
    qcompressedhelpinfo.h \
    qfilternamedialog_p.h \
    qhelpenginecore.h \
    qhelpengine.h \
    qhelpengine_p.h \
    qhelpfilterdata.h \
    qhelpfilterengine.h \
    qhelpfiltersettings_p.h \
    qhelpfiltersettingswidget.h \
    qhelp_global.h \
    qhelpdbreader_p.h \
    qhelpcontentwidget.h \
    qhelpindexwidget.h \
    qhelplink.h \
    qhelpcollectionhandler_p.h \
    qhelpsearchengine.h \
    qhelpsearchquerywidget.h \
    qhelpsearchresultwidget.h \
    qhelpsearchindexwriter_default_p.h \
    qhelpsearchindexreader_default_p.h \
    qhelpsearchindexreader_p.h \
    qoptionswidget_p.h

FORMS += \
    qhelpfiltersettingswidget.ui \
    qfilternamedialog.ui


load(qt_module)
