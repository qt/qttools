// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE

class QAction;
class QComboBox;
class QLineEdit;
class QMenu;

class CentralWidget;
class CmdLineParser;
class ContentWindow;
class IndexWindow;
class QtDocInstaller;
class SearchWidget;
struct QHelpLink;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_MOC_INCLUDE(<QtHelp/qhelplink.h>)

public:
    explicit MainWindow(CmdLineParser *cmdLine, QWidget *parent = nullptr);
    ~MainWindow() override;

    static void activateCurrentBrowser();
    static QString collectionFileDirectory(bool createDir = false,
        const QString &cacheDir = QString());
    static QString defaultHelpCollectionFileName();

public:
    void setIndexString(const QString &str);
    void expandTOC(int depth);
    bool usesDefaultCollection() const;

signals:
    void initDone();

public slots:
    void setContentsVisible(bool visible);
    void setIndexVisible(bool visible);
    void setBookmarksVisible(bool visible);
    void setSearchVisible(bool visible);
    void syncContents();
    void activateCurrentCentralWidgetTab();
    void currentFilterChanged(const QString &filter);

private slots:
    void showContents();
    void showIndex();
    void showSearch();
    void showOpenPages();
    void insertLastPages();
    void gotoAddress();
    void showPreferences();
    void showNewAddress();
    void showAboutDialog();
    void showNewAddress(const QUrl &url);
    void showTopicChooser(const QList<QHelpLink> &documents, const QString &keyword);
    void updateApplicationFont();
    void filterDocumentation(int filterIndex);
    void setupFilterCombo();
    void lookForNewQtDocumentation();
    void indexingStarted();
    void indexingFinished();
    void qtDocumentationInstalled();
    void registerDocumentation(const QString &component,
        const QString &absFileName);
    void resetQtDocInfo(const QString &component);
    void checkInitState();
    void documentationRemoved(const QString &namespaceName);
    void documentationUpdated(const QString &namespaceName);

private:
    bool initHelpDB(bool registerInternalDoc);
    void setupActions();
    void closeEvent(QCloseEvent *e) override;
    void activateDockWidget(QWidget *w);
    void updateAboutMenuText();
    void setupFilterToolbar();
    void setupAddressToolbar();
    QMenu *toolBarMenu();
    void hideContents();
    void hideIndex();
    void hideSearch();

private slots:
    void showBookmarksDockWidget();
    void hideBookmarksDockWidget();
    void handlePageCountChanged();

private:
    QWidget *m_bookmarkWidget = nullptr;
    CentralWidget *m_centralWidget;
    IndexWindow *m_indexWindow;
    ContentWindow *m_contentWindow;
    SearchWidget *m_searchWindow;
    QLineEdit *m_addressLineEdit;
    QComboBox *m_filterCombo = nullptr;

    QAction *m_syncAction;
    QAction *m_printPreviewAction;
    QAction *m_pageSetupAction;
    QAction *m_resetZoomAction;
    QAction *m_aboutAction;
    QAction *m_closeTabAction;
    QAction *m_newTabAction;

    QMenu *m_viewMenu;
    QMenu *m_toolBarMenu = nullptr;

    CmdLineParser *m_cmdLine;

    QWidget *m_progressWidget = nullptr;
    QtDocInstaller *m_qtDocInstaller = nullptr;

    bool m_connectedInitSignals = false;
};

QT_END_NAMESPACE

#endif // MAINWINDOW_H
