# Generated from assistant.pro.

#####################################################################
## assistant Binary:
#####################################################################

qt_add_executable(assistant
    GUI
    SOURCES
        ../../shared/fontpanel/fontpanel.cpp ../../shared/fontpanel/fontpanel.h
        ../shared/collectionconfiguration.cpp ../shared/collectionconfiguration.h
        aboutdialog.cpp aboutdialog.h
        bookmarkdialog.cpp bookmarkdialog.h bookmarkdialog.ui
        bookmarkfiltermodel.cpp bookmarkfiltermodel.h
        bookmarkitem.cpp bookmarkitem.h
        bookmarkmanager.cpp bookmarkmanager.h
        bookmarkmanagerwidget.cpp bookmarkmanagerwidget.h bookmarkmanagerwidget.ui
        bookmarkmodel.cpp bookmarkmodel.h
        bookmarkwidget.ui
        centralwidget.cpp centralwidget.h
        cmdlineparser.cpp cmdlineparser.h
        contentwindow.cpp contentwindow.h
        findwidget.cpp findwidget.h
        globalactions.cpp globalactions.h
        helpbrowsersupport.cpp helpbrowsersupport.h
        helpdocsettings.cpp helpdocsettings.h
        helpdocsettingswidget.cpp helpdocsettingswidget.h helpdocsettingswidget.ui
        helpenginewrapper.cpp helpenginewrapper.h
        helpviewer.cpp helpviewer.h helpviewer_p.h
        indexwindow.cpp indexwindow.h
        main.cpp
        mainwindow.cpp mainwindow.h
        openpagesmanager.cpp openpagesmanager.h
        openpagesmodel.cpp openpagesmodel.h
        openpagesswitcher.cpp openpagesswitcher.h
        openpageswidget.cpp openpageswidget.h
        preferencesdialog.cpp preferencesdialog.h preferencesdialog.ui
        qtdocinstaller.cpp qtdocinstaller.h
        remotecontrol.cpp remotecontrol.h
        searchwidget.cpp searchwidget.h
        topicchooser.cpp topicchooser.h topicchooser.ui
        tracer.h
        xbelsupport.cpp xbelsupport.h
    INCLUDE_DIRECTORIES
        ../../shared/fontpanel
    PUBLIC_LIBRARIES
        Qt::Gui
        Qt::Help
        Qt::Network
        Qt::Sql
        Qt::Widgets
        Qt::PrintSupport # special case
    ENABLE_AUTOGEN_TOOLS
        uic
)

# Resources:
set(assistant_resource_files
    "assistant.qch"
)

qt_add_resource(assistant "assistant"
    PREFIX
        "/qt-project.org/assistant"
    FILES
        ${assistant_resource_files}
)
set(assistant_images_resource_files
    "images/assistant-128.png"
    "images/assistant.png"
    "images/bookmark.png"
    "images/closebutton.png"
    "images/darkclosebutton.png"
    "images/mac/addtab.png"
    "images/mac/book.png"
    "images/mac/closetab.png"
    "images/mac/editcopy.png"
    "images/mac/find.png"
    "images/mac/home.png"
    "images/mac/next.png"
    "images/mac/previous.png"
    "images/mac/print.png"
    "images/mac/resetzoom.png"
    "images/mac/synctoc.png"
    "images/mac/zoomin.png"
    "images/mac/zoomout.png"
    "images/win/addtab.png"
    "images/win/book.png"
    "images/win/closetab.png"
    "images/win/editcopy.png"
    "images/win/find.png"
    "images/win/home.png"
    "images/win/next.png"
    "images/win/previous.png"
    "images/win/print.png"
    "images/win/resetzoom.png"
    "images/win/synctoc.png"
    "images/win/zoomin.png"
    "images/win/zoomout.png"
    "images/wrap.png"
)

qt_add_resource(assistant "assistant_images"
    PREFIX
        "/qt-project.org/assistant"
    FILES
        ${assistant_images_resource_files}
)


#### Keys ignored in scope 1:.:.:assistant.pro:<TRUE>:
# PROJECTNAME = "Assistant"
# QMAKE_TARGET_DESCRIPTION = "Qt" "Assistant"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:assistant.pro:TARGET Qt::WebKitWidgets AND NOT QT_CONFIG___contains___static:
# BROWSER = "qtwebkit"

#### Keys ignored in scope 3:.:.:assistant.pro:else:
# BROWSER = "qtextbrowser"

qt_extend_target(assistant CONDITION TARGET Qt::PrintSupport
    PUBLIC_LIBRARIES
        Qt::PrintSupport
)

qt_extend_target(assistant CONDITION BROWSER___equals___qtwebkit
    SOURCES
        helpviewer_qwv.cpp
    DEFINES
        BROWSER_QTWEBKIT
    PUBLIC_LIBRARIES
        Qt::WebKitWidgets
)

qt_extend_target(assistant CONDITION NOT BROWSER___equals___qtwebkit
    SOURCES
        helpviewer_qtb.cpp
    DEFINES
        BROWSER_QTEXTBROWSER
)

qt_extend_target(assistant CONDITION WIN32
    SOURCES
        stdinlistener_win.cpp stdinlistener_win.h
    PUBLIC_LIBRARIES
        shell32
)

qt_extend_target(assistant CONDITION UNIX
    SOURCES
        stdinlistener.cpp stdinlistener.h
)

#### Keys ignored in scope 11:.:.:assistant.pro:APPLE:
# ICON = "assistant.icns"
# QMAKE_INFO_PLIST = "Info_mac.plist"
qt_add_docs(assistant
    doc/qtassistant.qdocconf
)

