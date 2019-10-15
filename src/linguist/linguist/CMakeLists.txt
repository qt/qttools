# Generated from linguist.pro.

#####################################################################
## linguist Binary:
#####################################################################

add_qt_executable(linguist
    GUI
    SOURCES
        ../shared/numerus.cpp
        ../shared/po.cpp
        ../shared/qm.cpp
        ../shared/qph.cpp
        ../shared/simtexth.cpp ../shared/simtexth.h
        ../shared/translator.cpp ../shared/translator.h
        ../shared/translatormessage.cpp ../shared/translatormessage.h
        ../shared/ts.cpp
        ../shared/xliff.cpp
        batchtranslation.ui
        batchtranslationdialog.cpp batchtranslationdialog.h
        errorsview.cpp errorsview.h
        finddialog.cpp finddialog.h finddialog.ui
        formpreviewview.cpp formpreviewview.h
        globals.cpp globals.h
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        messageeditor.cpp messageeditor.h
        messageeditorwidgets.cpp messageeditorwidgets.h
        messagehighlighter.cpp messagehighlighter.h
        messagemodel.cpp messagemodel.h
        phrase.cpp phrase.h
        phrasebookbox.cpp phrasebookbox.h phrasebookbox.ui
        phrasemodel.cpp phrasemodel.h
        phraseview.cpp phraseview.h
        printout.cpp printout.h
        recentfiles.cpp recentfiles.h
        sourcecodeview.cpp sourcecodeview.h
        statistics.cpp statistics.h statistics.ui
        translatedialog.cpp translatedialog.h translatedialog.ui
        translationsettings.ui
        translationsettingsdialog.cpp translationsettingsdialog.h
    DEFINES
        QFORMINTERNAL_NAMESPACE
        QT_KEYWORDS
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::UiToolsPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::PrintSupport
        Qt::UiTools
        Qt::Widgets
        Qt::Xml
    ENABLE_AUTOGEN_TOOLS
        uic
)

# Resources:
set(linguist_resource_files
    "images/appicon.png"
    "images/down.png"
    "images/editdelete.png"
    "images/icons/linguist-128-32.png"
    "images/mac/accelerator.png"
    "images/mac/book.png"
    "images/mac/done.png"
    "images/mac/doneandnext.png"
    "images/mac/editcopy.png"
    "images/mac/editcut.png"
    "images/mac/editpaste.png"
    "images/mac/fileopen.png"
    "images/mac/filesave.png"
    "images/mac/next.png"
    "images/mac/nextunfinished.png"
    "images/mac/phrase.png"
    "images/mac/prev.png"
    "images/mac/prevunfinished.png"
    "images/mac/print.png"
    "images/mac/punctuation.png"
    "images/mac/redo.png"
    "images/mac/searchfind.png"
    "images/mac/surroundingwhitespace.png"
    "images/mac/undo.png"
    "images/mac/validateplacemarkers.png"
    "images/mac/whatsthis.png"
    "images/minus.png"
    "images/plus.png"
    "images/s_check_danger.png"
    "images/s_check_empty.png"
    "images/s_check_obsolete.png"
    "images/s_check_off.png"
    "images/s_check_on.png"
    "images/s_check_warning.png"
    "images/up.png"
    "images/win/accelerator.png"
    "images/win/book.png"
    "images/win/done.png"
    "images/win/doneandnext.png"
    "images/win/editcopy.png"
    "images/win/editcut.png"
    "images/win/editpaste.png"
    "images/win/fileopen.png"
    "images/win/filesave.png"
    "images/win/next.png"
    "images/win/nextunfinished.png"
    "images/win/phrase.png"
    "images/win/prev.png"
    "images/win/prevunfinished.png"
    "images/win/print.png"
    "images/win/punctuation.png"
    "images/win/redo.png"
    "images/win/searchfind.png"
    "images/win/surroundingwhitespace.png"
    "images/win/undo.png"
    "images/win/validateplacemarkers.png"
    "images/win/whatsthis.png"
)

add_qt_resource(linguist "linguist"
    PREFIX
        "/"
    FILES
        ${linguist_resource_files}
)


#### Keys ignored in scope 1:.:.:linguist.pro:<TRUE>:
# PROJECTNAME = "Qt" "Linguist"
# QMAKE_TARGET_DESCRIPTION = "Qt" "Linguist"
# _LOADED = "qt_app"
# phrasebooks.files = "$$PWD/../phrasebooks/*"
# phrasebooks.path = "$$[QT_INSTALL_DATA]/phrasebooks"

## Scopes:
#####################################################################

extend_target(linguist CONDITION QT_PRODUCT___contains___OpenSource._x_
    DEFINES
        QT_OPENSOURCE
)

#### Keys ignored in scope 3:.:.:linguist.pro:WIN32:
# RC_ICONS = "linguist.ico"
# VERSION = "$${QT_VERSION}.0"

#### Keys ignored in scope 4:.:.:linguist.pro:else:
# VERSION = "$${QT_VERSION}"

#### Keys ignored in scope 5:.:.:linguist.pro:APPLE:
# ICON = "linguist.icns"
# QMAKE_INFO_PLIST = "Info_mac.plist"
add_qt_docs(linguist
    doc/qtlinguist.qdocconf
)

