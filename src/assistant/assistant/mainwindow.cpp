// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mainwindow.h"

#include "aboutdialog.h"
#include "bookmarkmanager.h"
#include "centralwidget.h"
#include "cmdlineparser.h"
#include "contentwindow.h"
#include "globalactions.h"
#include "helpenginewrapper.h"
#include "indexwindow.h"
#include "openpagesmanager.h"
#include "preferencesdialog.h"
#include "qtdocinstaller.h"
#include "remotecontrol.h"
#include "searchwidget.h"
#include "topicchooser.h"
#include "tracer.h"

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPair>
#include <QtCore/QResource>
#include <QtCore/QStandardPaths>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <QtCore/QBuffer>
#include <QtCore/QLibraryInfo>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <QtGui/QAction>
#include <QtGui/QFontDatabase>
#include <QtGui/QImageReader>
#include <QtGui/QScreen>
#include <QtGui/QShortcut>

#include <QtWidgets/QComboBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>

#include <QtHelp/QHelpContentModel>
#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpIndexModel>
#include <QtHelp/QHelpSearchEngine>
#include <QtHelp/QHelpFilterEngine>

#include <cstdlib>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

enum { warnAboutMissingModules = 0 };

MainWindow::MainWindow(CmdLineParser *cmdLine, QWidget *parent)
    : QMainWindow(parent)
    , m_cmdLine(cmdLine)
{
    TRACE_OBJ

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setDockOptions(dockOptions() | AllowNestedDocks);

    QString collectionFile;
    if (usesDefaultCollection()) {
        MainWindow::collectionFileDirectory(true);
        collectionFile = MainWindow::defaultHelpCollectionFileName();
    } else {
        collectionFile = cmdLine->collectionFile();
    }
    HelpEngineWrapper &helpEngineWrapper =
        HelpEngineWrapper::instance(collectionFile);
    BookmarkManager *bookMarkManager = BookmarkManager::instance();

    if (!initHelpDB(!cmdLine->collectionFileGiven())) {
        qDebug("Fatal error: Help engine initialization failed. "
            "Error message was: %s\nAssistant will now exit.",
            qPrintable(HelpEngineWrapper::instance().error()));
        std::exit(1);
    }

    m_centralWidget = new CentralWidget(this);
    setCentralWidget(m_centralWidget);

    m_indexWindow = new IndexWindow(this);
    QDockWidget *indexDock = new QDockWidget(tr("Index"), this);
    indexDock->setObjectName("IndexWindow"_L1);
    indexDock->setWidget(m_indexWindow);
    addDockWidget(Qt::LeftDockWidgetArea, indexDock);

    m_contentWindow = new ContentWindow;
    QDockWidget *contentDock = new QDockWidget(tr("Contents"), this);
    contentDock->setObjectName("ContentWindow"_L1);
    contentDock->setWidget(m_contentWindow);
    addDockWidget(Qt::LeftDockWidgetArea, contentDock);

    m_searchWindow = new SearchWidget(helpEngineWrapper.searchEngine());
    m_searchWindow->setFont(!helpEngineWrapper.usesBrowserFont() ? qApp->font()
        : helpEngineWrapper.browserFont());
    QDockWidget *searchDock = new QDockWidget(tr("Search"), this);
    searchDock->setObjectName("SearchWindow"_L1);
    searchDock->setWidget(m_searchWindow);
    addDockWidget(Qt::LeftDockWidgetArea, searchDock);

    QDockWidget *bookmarkDock = new QDockWidget(tr("Bookmarks"), this);
    bookmarkDock->setObjectName("BookmarkWindow"_L1);
    bookmarkDock->setWidget(m_bookmarkWidget
        = bookMarkManager->bookmarkDockWidget());
    addDockWidget(Qt::LeftDockWidgetArea, bookmarkDock);

    QDockWidget *openPagesDock = new QDockWidget(tr("Open Pages"), this);
    openPagesDock->setObjectName("Open Pages"_L1);
    OpenPagesManager *openPagesManager
        = OpenPagesManager::createInstance(this, usesDefaultCollection(), m_cmdLine->url());
    openPagesDock->setWidget(openPagesManager->openPagesWidget());
    addDockWidget(Qt::LeftDockWidgetArea, openPagesDock);

    connect(m_centralWidget, &CentralWidget::addBookmark,
            bookMarkManager, &BookmarkManager::addBookmark);
    connect(bookMarkManager, &BookmarkManager::escapePressed,
            this, &MainWindow::activateCurrentCentralWidgetTab);
    connect(bookMarkManager, &BookmarkManager::setSource,
            m_centralWidget, &CentralWidget::setSource);
    connect(bookMarkManager, &BookmarkManager::setSourceInNewTab,
            openPagesManager, [openPagesManager](const QUrl &url){ openPagesManager->createPage(url); });

    QHelpSearchEngine *searchEngine = helpEngineWrapper.searchEngine();
    connect(searchEngine, &QHelpSearchEngine::indexingStarted,
            this, &MainWindow::indexingStarted);
    connect(searchEngine, &QHelpSearchEngine::indexingFinished,
            this, &MainWindow::indexingFinished);

    QString defWindowTitle = tr("Qt Assistant");
    setWindowTitle(defWindowTitle);

    setupActions();
    statusBar()->show();
    m_centralWidget->connectTabBar();

    setupFilterToolbar();
    setupAddressToolbar();

    const QString windowTitle = helpEngineWrapper.windowTitle();
    setWindowTitle(windowTitle.isEmpty() ? defWindowTitle : windowTitle);
    QByteArray iconArray = helpEngineWrapper.applicationIcon();
    if (iconArray.size() > 0) {
        QBuffer buffer(&iconArray);
        QImageReader reader(&buffer);
        QIcon appIcon;
        do {
            QPixmap pix;
            pix.convertFromImage(reader.read());
            appIcon.addPixmap(pix);
        } while (reader.jumpToNextImage());
        qApp->setWindowIcon(appIcon);
    }
#if !defined(Q_OS_MACOS) && !defined(Q_OS_WIN)
    else {
        QIcon appIcon(":/qt-project.org/assistant/images/assistant-128.png"_L1);
        qApp->setWindowIcon(appIcon);
    }
#endif

    QToolBar *toolBar = addToolBar(tr("Bookmark Toolbar"));
    toolBar->setObjectName("Bookmark Toolbar"_L1);
    bookMarkManager->setBookmarksToolbar(toolBar);

    toolBar->hide();
    toolBarMenu()->addAction(toolBar->toggleViewAction());

    QByteArray ba(helpEngineWrapper.mainWindow());
    if (!ba.isEmpty())
        restoreState(ba);

    ba = helpEngineWrapper.mainWindowGeometry();
    if (!ba.isEmpty()) {
        restoreGeometry(ba);
    } else {
        tabifyDockWidget(contentDock, indexDock);
        tabifyDockWidget(indexDock, bookmarkDock);
        tabifyDockWidget(bookmarkDock, searchDock);
        contentDock->raise();
        const QRect screen = QGuiApplication::primaryScreen()->geometry();
        adjustSize();   // make sure we won't start outside of the screen
        resize(4 * screen.width() / 5, 4 * screen.height() / 5);

        move(screen.center() - rect().center());
    }

    if (!helpEngineWrapper.hasFontSettings()) {
        helpEngineWrapper.setUseAppFont(false);
        helpEngineWrapper.setUseBrowserFont(false);
        helpEngineWrapper.setAppFont(qApp->font());
        helpEngineWrapper.setAppWritingSystem(QFontDatabase::Latin);
        helpEngineWrapper.setBrowserFont(qApp->font());
        helpEngineWrapper.setBrowserWritingSystem(QFontDatabase::Latin);
    } else {
        updateApplicationFont();
    }

    updateAboutMenuText();

    QTimer::singleShot(0, this, &MainWindow::insertLastPages);
    if (m_cmdLine->enableRemoteControl())
        (void)new RemoteControl(this);

    if (m_cmdLine->contents() == CmdLineParser::Show)
        showContents();
    else if (m_cmdLine->contents() == CmdLineParser::Hide)
        hideContents();

    if (m_cmdLine->index() == CmdLineParser::Show)
        showIndex();
    else if (m_cmdLine->index() == CmdLineParser::Hide)
        hideIndex();

    if (m_cmdLine->bookmarks() == CmdLineParser::Show)
        showBookmarksDockWidget();
    else if (m_cmdLine->bookmarks() == CmdLineParser::Hide)
        hideBookmarksDockWidget();

    if (m_cmdLine->search() == CmdLineParser::Show)
        showSearch();
    else if (m_cmdLine->search() == CmdLineParser::Hide)
        hideSearch();

    if (m_cmdLine->contents() == CmdLineParser::Activate)
        showContents();
    else if (m_cmdLine->index() == CmdLineParser::Activate)
        showIndex();
    else if (m_cmdLine->bookmarks() == CmdLineParser::Activate)
        showBookmarksDockWidget();

    if (!m_cmdLine->currentFilter().isEmpty()) {
        const QString &curFilter = m_cmdLine->currentFilter();
        if (helpEngineWrapper.filterEngine()->filters().contains(curFilter))
            helpEngineWrapper.filterEngine()->setActiveFilter(curFilter);
    }

    if (usesDefaultCollection())
        QTimer::singleShot(0, this, &MainWindow::lookForNewQtDocumentation);
    else
        checkInitState();

    connect(&helpEngineWrapper, &HelpEngineWrapper::documentationRemoved,
            this, &MainWindow::documentationRemoved);
    connect(&helpEngineWrapper, &HelpEngineWrapper::documentationUpdated,
            this, &MainWindow::documentationUpdated);

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    GlobalActions::instance()->updateActions();
    if (helpEngineWrapper.addressBarEnabled())
        showNewAddress();
}

MainWindow::~MainWindow()
{
    TRACE_OBJ
    delete m_qtDocInstaller;
}

bool MainWindow::usesDefaultCollection() const
{
    TRACE_OBJ
    return m_cmdLine->collectionFile().isEmpty();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    TRACE_OBJ
    BookmarkManager::destroy();
    HelpEngineWrapper::instance().setMainWindow(saveState());
    HelpEngineWrapper::instance().setMainWindowGeometry(saveGeometry());
    QMainWindow::closeEvent(e);
}

bool MainWindow::initHelpDB(bool registerInternalDoc)
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngineWrapper = HelpEngineWrapper::instance();
    if (!helpEngineWrapper.setupData())
        return false;

    if (!registerInternalDoc && helpEngineWrapper.defaultHomePage() == "help"_L1)
        helpEngineWrapper.setDefaultHomePage("about:blank"_L1);

    return true;
}

static const char *docs[] = {
    "assistant", "designer", "linguist", // Qt 4
    "qmake",
    "qt",
    "qtqmake",
    "activeqt",
    "qtandroidextras",
    "qtassistant",
    "qtbluetooth",
    "qtconcurrent",
    "qtconnectivity",
    "qtcore",
    "qtdbus",
    "qtdesigner",
    "qtdoc",
    "qtenginio",
    "qtgraphicaleffects",
    "qtgui",
    "qthelp",
    "qtimageformats",
    "qtlinguist",
    "qtlocation",
    "qtmacextras",
    "qtmultimedia",
    "qtmultimediawidgets",
    "qtnfc",
    "qtnetwork",
    "qtopengl",
    "qtpositioning",
    "qtprintsupport",
    "qtqml",
    "qtquick",
    "qtscript",
    "qtscripttools",
    "qtsensors",
    "qtsql",
    "qtsvg",
    "qttestlib",
    "qtuitools",
    "qtwebkit",
    "qtwebkitexamples",
    "qtwidgets",
    "qtxml",
    "qtxmlpatterns",
    "qdoc",
    "qtx11extras",
    "qtserialport",
    "qtquickcontrols",
    "qtquickcontrolsstyles",
    "qtquickdialogs",
    "qtquicklayouts",
    "qtwebsockets",
    "qtwinextras"
};

static QStringList newQtDocumentation()
{
    QStringList result;
    const QDir docDirectory(QLibraryInfo::path(QLibraryInfo::DocumentationPath));
    const QFileInfoList entries = docDirectory.entryInfoList(QStringList(QStringLiteral("*.qch")),
                                                             QDir::Files, QDir::Name);
    if (!entries.isEmpty()) {
        result.reserve(entries.size());
        for (const QFileInfo &fi : entries)
            result.append(fi.baseName());
        return result;
    }
    if (warnAboutMissingModules)
        qWarning() << "No documentation found in " << QDir::toNativeSeparators(docDirectory.absolutePath());
    const int docCount = int(sizeof(docs) / sizeof(docs[0]));
    result.reserve(docCount);
    for (int d = 0; d < docCount; ++d)
        result.append(QLatin1StringView(docs[d]));
    return result;
}

void MainWindow::lookForNewQtDocumentation()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();

    const QStringList &docs = newQtDocumentation();
    const int docCount = docs.size();
    QList<QtDocInstaller::DocInfo> qtDocInfos;
    qtDocInfos.reserve(docCount);
    for (const QString &doc : docs) {
        const QtDocInstaller::DocInfo docInfo(doc, helpEngine.qtDocInfo(doc));
        qtDocInfos.append(docInfo);
        if (warnAboutMissingModules && (docInfo.second.isEmpty() || docInfo.second.first().isEmpty()))
            qWarning() << "No documentation found for " << doc;
    }

    m_qtDocInstaller = new QtDocInstaller(qtDocInfos);
    connect(m_qtDocInstaller, &QtDocInstaller::docsInstalled,
            this, &MainWindow::qtDocumentationInstalled);
    connect(m_qtDocInstaller, &QtDocInstaller::qchFileNotFound,
            this, &MainWindow::resetQtDocInfo);
    connect(m_qtDocInstaller, &QtDocInstaller::registerDocumentation,
            this, &MainWindow::registerDocumentation);
    if (helpEngine.qtDocInfo("qt"_L1).size() != 2)
        statusBar()->showMessage(tr("Looking for Qt Documentation..."));
    m_qtDocInstaller->installDocs();
}

void MainWindow::qtDocumentationInstalled()
{
    TRACE_OBJ
    OpenPagesManager::instance()->resetHelpPage();
    statusBar()->clearMessage();
    checkInitState();
}

void MainWindow::checkInitState()
{
    TRACE_OBJ
    if (!m_cmdLine->enableRemoteControl()) {
        HelpEngineWrapper::instance().initialDocSetupDone();
        return;
    }

    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (helpEngine.contentModel()->isCreatingContents()
        || helpEngine.indexModel()->isCreatingIndex()) {
        if (!m_connectedInitSignals) {
            connect(helpEngine.contentModel(), &QHelpContentModel::contentsCreated,
                    this, &MainWindow::checkInitState);
            connect(helpEngine.indexModel(), &QHelpIndexModel::indexCreated,
                    this, &MainWindow::checkInitState);
            m_connectedInitSignals = true;
        }
    } else {
        if (m_connectedInitSignals) {
            disconnect(helpEngine.contentModel(), nullptr, this, nullptr);
            disconnect(helpEngine.indexModel(), nullptr, this, nullptr);
        }
        HelpEngineWrapper::instance().initialDocSetupDone();
        emit initDone();
    }
}

void MainWindow::insertLastPages()
{
    TRACE_OBJ
    if (m_cmdLine->search() == CmdLineParser::Activate)
        showSearch();
}

void MainWindow::setupActions()
{
    TRACE_OBJ
    QString resourcePath = ":/qt-project.org/assistant/images/"_L1;
#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
    resourcePath.append("mac"_L1);
#else
    resourcePath.append("win"_L1);
#endif

    QMenu *menu = menuBar()->addMenu(tr("&File"));
    OpenPagesManager *const openPages = OpenPagesManager::instance();
    m_newTabAction = menu->addAction(tr("New &Tab"),
            openPages, &OpenPagesManager::createBlankPage);
    m_newTabAction->setShortcut(QKeySequence::AddTab);
    m_closeTabAction = menu->addAction(tr("&Close Tab"),
            openPages, &OpenPagesManager::closeCurrentPage);
    m_closeTabAction->setShortcuts(QKeySequence::Close);
    m_closeTabAction->setEnabled(openPages->pageCount() > 1);
    connect(openPages, &OpenPagesManager::pageClosed,
            this, &MainWindow::handlePageCountChanged);
    connect(openPages, &OpenPagesManager::pageAdded,
            this, &MainWindow::handlePageCountChanged);

    menu->addSeparator();

    m_pageSetupAction = menu->addAction(tr("Page Set&up..."),
            m_centralWidget, &CentralWidget::pageSetup);
    m_printPreviewAction = menu->addAction(tr("Print Preview..."),
            m_centralWidget, &CentralWidget::printPreview);

    GlobalActions *globalActions = GlobalActions::instance(this);
    menu->addAction(globalActions->printAction());
    menu->addSeparator();

    QIcon appExitIcon = QIcon::fromTheme(QIcon::ThemeIcon::ApplicationExit);
    QAction *tmp;
#ifdef Q_OS_WIN
    tmp = menu->addAction(appExitIcon, tr("E&xit"),
            this, &QWidget::close);
    tmp->setShortcut(QKeySequence(tr("CTRL+Q")));
#else
    tmp = menu->addAction(appExitIcon, tr("&Quit"),
            this, &QWidget::close);
    tmp->setShortcut(QKeySequence::Quit);
#endif
    tmp->setMenuRole(QAction::QuitRole);

    menu = menuBar()->addMenu(tr("&Edit"));
#if QT_CONFIG(clipboard)
    menu->addAction(globalActions->copyAction());
#endif
    menu->addAction(globalActions->findAction());

    QAction *findNextAction = menu->addAction(tr("Find &Next"),
            m_centralWidget, &CentralWidget::findNext);
    findNextAction->setShortcuts(QKeySequence::FindNext);

    QAction *findPreviousAction = menu->addAction(tr("Find &Previous"),
            m_centralWidget, &CentralWidget::findPrevious);
    findPreviousAction->setShortcuts(QKeySequence::FindPrevious);

    menu->addSeparator();
    tmp = menu->addAction(tr("Preferences..."),
            this, &MainWindow::showPreferences);
    tmp->setMenuRole(QAction::PreferencesRole);

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(globalActions->zoomInAction());
    m_viewMenu->addAction(globalActions->zoomOutAction());

    m_resetZoomAction = m_viewMenu->addAction(tr("Normal &Size"),
            m_centralWidget, &CentralWidget::resetZoom);
    m_resetZoomAction->setPriority(QAction::LowPriority);
    m_resetZoomAction->setIcon(QIcon(resourcePath + "/resetzoom.png"_L1));
    m_resetZoomAction->setShortcut(tr("Ctrl+0"));

    m_viewMenu->addSeparator();

    m_viewMenu->addAction(tr("Contents"), QKeySequence(tr("ALT+C")),
                          this, &MainWindow::showContents);
    m_viewMenu->addAction(tr("Index"), QKeySequence(tr("ALT+I")),
                          this, &MainWindow::showIndex);
    m_viewMenu->addAction(tr("Bookmarks"), QKeySequence(tr("ALT+O")),
                          this, &MainWindow::showBookmarksDockWidget);
    m_viewMenu->addAction(tr("Search"), QKeySequence(tr("ALT+S")),
                          this, &MainWindow::showSearch);
    m_viewMenu->addAction(tr("Open Pages"), QKeySequence(tr("ALT+P")),
                          this, &MainWindow::showOpenPages);

    menu = menuBar()->addMenu(tr("&Go"));
    menu->addAction(globalActions->homeAction());
    menu->addAction(globalActions->backAction());
    menu->addAction(globalActions->nextAction());

    m_syncAction = menu->addAction(tr("Sync with Table of Contents"),
            this, &MainWindow::syncContents);
    m_syncAction->setIconText(tr("Sync"));
    m_syncAction->setIcon(QIcon(resourcePath + "/synctoc.png"_L1));

    menu->addSeparator();

    tmp = menu->addAction(tr("Next Page"),
            openPages, &OpenPagesManager::nextPage);
    tmp->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Alt+Right"))
        << QKeySequence(Qt::CTRL | Qt::Key_PageDown));

    tmp = menu->addAction(tr("Previous Page"),
            openPages, &OpenPagesManager::previousPage);
    tmp->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Alt+Left"))
        << QKeySequence(Qt::CTRL | Qt::Key_PageUp));

    const Qt::Modifier modifier =
#ifdef Q_OS_MAC
            Qt::ALT;
#else
            Qt::CTRL;
#endif

    QShortcut *sct = new QShortcut(QKeySequence(modifier | Qt::Key_Tab), this);
    connect(sct, &QShortcut::activated,
            openPages, &OpenPagesManager::nextPageWithSwitcher);
    sct = new QShortcut(QKeySequence(modifier | Qt::SHIFT | Qt::Key_Tab), this);
    connect(sct, &QShortcut::activated,
            openPages, &OpenPagesManager::previousPageWithSwitcher);

    BookmarkManager::instance()->setBookmarksMenu(menuBar()->addMenu(tr("&Bookmarks")));

    menu = menuBar()->addMenu(tr("&Help"));
    m_aboutAction = menu->addAction(tr("About..."),
            this, &MainWindow::showAboutDialog);
    m_aboutAction->setMenuRole(QAction::AboutRole);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    m_resetZoomAction->setIcon(QIcon::fromTheme("zoom-original"_L1, m_resetZoomAction->icon()));
    m_syncAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewRefresh,
                          m_syncAction->icon()));
#endif

    QToolBar *navigationBar = addToolBar(tr("Navigation Toolbar"));
    navigationBar->setObjectName("NavigationToolBar"_L1);
    navigationBar->addAction(globalActions->backAction());
    navigationBar->addAction(globalActions->nextAction());
    navigationBar->addAction(globalActions->homeAction());
    navigationBar->addAction(m_syncAction);
    navigationBar->addSeparator();
#if QT_CONFIG(clipboard)
    navigationBar->addAction(globalActions->copyAction());
#endif
    navigationBar->addAction(globalActions->printAction());
    navigationBar->addAction(globalActions->findAction());
    navigationBar->addSeparator();
    navigationBar->addAction(globalActions->zoomInAction());
    navigationBar->addAction(globalActions->zoomOutAction());
    navigationBar->addAction(m_resetZoomAction);

#if defined(Q_OS_MAC)
    QMenu *windowMenu = new QMenu(tr("&Window"), this);
    menuBar()->insertMenu(menu->menuAction(), windowMenu);
    windowMenu->addAction(tr("Zoom"),
            this, &QWidget::showMaximized);
    windowMenu->addAction(tr("Minimize"), QKeySequence(tr("Ctrl+M")),
            this, &QWidget::showMinimized);
#endif

    // content viewer connections
#if QT_CONFIG(clipboard)
    connect(m_centralWidget, &CentralWidget::copyAvailable,
            globalActions, &GlobalActions::setCopyAvailable);
#endif
    connect(m_centralWidget, &CentralWidget::currentViewerChanged,
            globalActions, &GlobalActions::updateActions);
    connect(m_centralWidget, &CentralWidget::forwardAvailable,
            globalActions, &GlobalActions::updateActions);
    connect(m_centralWidget, &CentralWidget::backwardAvailable,
            globalActions, &GlobalActions::updateActions);
    connect(m_centralWidget, &CentralWidget::highlighted,
            this, [this](const QUrl &link) { statusBar()->showMessage(link.toString());} );

    // index window
    connect(m_indexWindow, &IndexWindow::linkActivated,
            m_centralWidget, &CentralWidget::setSource);
    connect(m_indexWindow, &IndexWindow::documentsActivated,
            this, &MainWindow::showTopicChooser);
    connect(m_indexWindow, &IndexWindow::escapePressed,
            this, &MainWindow::activateCurrentCentralWidgetTab);

    // content window
    connect(m_contentWindow, &ContentWindow::linkActivated,
            m_centralWidget, &CentralWidget::setSource);
    connect(m_contentWindow, &ContentWindow::escapePressed,
            this, &MainWindow::activateCurrentCentralWidgetTab);

    // search window
    connect(m_searchWindow, &SearchWidget::requestShowLink,
            CentralWidget::instance(), &CentralWidget::setSourceFromSearch);
    connect(m_searchWindow, &SearchWidget::requestShowLinkInNewTab,
            OpenPagesManager::instance(), &OpenPagesManager::createNewPageFromSearch);

#if defined(QT_NO_PRINTER)
        m_pageSetupAction->setVisible(false);
        m_printPreviewAction->setVisible(false);
        globalActions->printAction()->setVisible(false);
#endif
}

QMenu *MainWindow::toolBarMenu()
{
    TRACE_OBJ
    if (!m_toolBarMenu) {
        m_viewMenu->addSeparator();
        m_toolBarMenu = m_viewMenu->addMenu(tr("Toolbars"));
    }
    return m_toolBarMenu;
}

void MainWindow::setupFilterToolbar()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (!helpEngine.filterFunctionalityEnabled())
        return;

    m_filterCombo = new QComboBox(this);
    m_filterCombo->setMinimumWidth(QFontMetrics({}).
        horizontalAdvance("MakeTheComboBoxWidthEnough"_L1));

    QToolBar *filterToolBar = addToolBar(tr("Filter Toolbar"));
    filterToolBar->setObjectName("FilterToolBar"_L1);
    filterToolBar->addWidget(new QLabel(tr("Filtered by:").append(u' '),
        this));
    filterToolBar->addWidget(m_filterCombo);

    if (!helpEngine.filterToolbarVisible())
        filterToolBar->hide();
    toolBarMenu()->addAction(filterToolBar->toggleViewAction());

    connect(&helpEngine, &HelpEngineWrapper::setupFinished,
            this, &MainWindow::setupFilterCombo, Qt::QueuedConnection);
    connect(m_filterCombo, &QComboBox::activated,
            this, &MainWindow::filterDocumentation);
    connect(helpEngine.filterEngine(), &QHelpFilterEngine::filterActivated,
            this, &MainWindow::currentFilterChanged);

    setupFilterCombo();
}

void MainWindow::setupAddressToolbar()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (!helpEngine.addressBarEnabled())
        return;

    m_addressLineEdit = new QLineEdit(this);
    QToolBar *addressToolBar = addToolBar(tr("Address Toolbar"));
    addressToolBar->setObjectName("AddressToolBar"_L1);
    insertToolBarBreak(addressToolBar);

    addressToolBar->addWidget(new QLabel(tr("Address:").append(QChar::Space),
        this));
    addressToolBar->addWidget(m_addressLineEdit);

    if (!helpEngine.addressBarVisible())
        addressToolBar->hide();
    toolBarMenu()->addAction(addressToolBar->toggleViewAction());

    // address lineedit
    connect(m_addressLineEdit, &QLineEdit::returnPressed, this, &MainWindow::gotoAddress);
    connect(m_centralWidget, &CentralWidget::currentViewerChanged,
            this, QOverload<>::of(&MainWindow::showNewAddress));
    connect(m_centralWidget, &CentralWidget::sourceChanged,
            this, QOverload<>::of(&MainWindow::showNewAddress));
}

void MainWindow::updateAboutMenuText()
{
    TRACE_OBJ
    QByteArray ba = HelpEngineWrapper::instance().aboutMenuTexts();
    if (ba.size() > 0) {
        QString lang;
        QString str;
        QString trStr;
        QString currentLang = QLocale::system().name();
        int i = currentLang.indexOf(u'_');
        if (i > -1)
            currentLang = currentLang.left(i);
        QDataStream s(&ba, QIODevice::ReadOnly);
        while (!s.atEnd()) {
            s >> lang;
            s >> str;
            if (lang == "default"_L1 && trStr.isEmpty()) {
                trStr = str;
            } else if (lang == currentLang) {
                trStr = str;
                break;
            }
        }
        if (!trStr.isEmpty())
            m_aboutAction->setText(trStr);
    }
}

void MainWindow::showNewAddress()
{
    TRACE_OBJ
    showNewAddress(m_centralWidget->currentSource());
}

void MainWindow::showNewAddress(const QUrl &url)
{
    TRACE_OBJ
    m_addressLineEdit->setText(url.toString());
}

void MainWindow::gotoAddress()
{
    TRACE_OBJ
    m_centralWidget->setSource(m_addressLineEdit->text());
}

void MainWindow::showTopicChooser(const QList<QHelpLink> &documents,
                                  const QString &keyword)
{
    TRACE_OBJ
    TopicChooser tc(this, keyword, documents);
    if (tc.exec() == QDialog::Accepted) {
        m_centralWidget->setSource(tc.link());
    }
}

void MainWindow::showPreferences()
{
    TRACE_OBJ
    PreferencesDialog dia(this);
    connect(&dia, &PreferencesDialog::updateApplicationFont,
            this, &MainWindow::updateApplicationFont);
    connect(&dia, &PreferencesDialog::updateBrowserFont,
            m_centralWidget, &CentralWidget::updateBrowserFont);
    connect(&dia, &PreferencesDialog::updateUserInterface,
            m_centralWidget, &CentralWidget::updateUserInterface);
    dia.exec();
}

void MainWindow::syncContents()
{
    TRACE_OBJ
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    const QUrl url = m_centralWidget->currentSource();
    showContents();
    if (!m_contentWindow->syncToContent(url))
        statusBar()->showMessage(
            tr("Could not find the associated content item."), 3000);
    qApp->restoreOverrideCursor();
}

void MainWindow::showAboutDialog()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    QByteArray contents;
    QByteArray ba = helpEngine.aboutTexts();
    if (!ba.isEmpty()) {
        QString lang;
        QByteArray cba;
        QString currentLang = QLocale::system().name();
        int i = currentLang.indexOf(u'_');
        if (i > -1)
            currentLang = currentLang.left(i);
        QDataStream s(&ba, QIODevice::ReadOnly);
        while (!s.atEnd()) {
            s >> lang;
            s >> cba;
            if (lang == "default"_L1 && contents.isEmpty()) {
                contents = cba;
            } else if (lang == currentLang) {
                contents = cba;
                break;
            }
        }
    }

    AboutDialog aboutDia(this);

    QByteArray iconArray;
    if (!contents.isEmpty()) {
        iconArray = helpEngine.aboutIcon();
        QByteArray resources = helpEngine.aboutImages();
        QPixmap pix;
        pix.loadFromData(iconArray);
        aboutDia.setText(QString::fromUtf8(contents), resources);
        if (!pix.isNull())
            aboutDia.setPixmap(pix);
        aboutDia.setWindowTitle(aboutDia.documentTitle());
    } else {
        QByteArray resources;
#if defined(BROWSER_QTWEBKIT)
        QString browser = QStringLiteral("Qt WebKit");
#else
        QString browser = QStringLiteral("QTextBrowser");
#endif
        if (m_centralWidget->currentHelpViewer())
            browser = QStringLiteral("QLiteHtmlWidget");
        aboutDia.setText(tr("<center>"
            "<h3>%1</h3>"
            "<p>Version %2</p>"
            "<p>Browser: %3</p></center>"
            "<p>Copyright (C) The Qt Company Ltd. and other contributors.</p>")
            .arg(tr("Qt Assistant"), QLatin1String(QT_VERSION_STR), browser),
            resources);
        aboutDia.setPixmap(QString(":/qt-project.org/assistant/images/assistant-128.png"_L1));
    }
    if (aboutDia.windowTitle().isEmpty())
        aboutDia.setWindowTitle(tr("About %1").arg(windowTitle()));
    aboutDia.exec();
}

void MainWindow::setContentsVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showContents();
    else
        hideContents();
}

void MainWindow::showContents()
{
    TRACE_OBJ
    activateDockWidget(m_contentWindow);
}

void MainWindow::hideContents()
{
    TRACE_OBJ
    m_contentWindow->parentWidget()->hide();
}

void MainWindow::setIndexVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showIndex();
    else
        hideIndex();
}

void MainWindow::showIndex()
{
    TRACE_OBJ
    activateDockWidget(m_indexWindow);
}

void MainWindow::hideIndex()
{
    TRACE_OBJ
    m_indexWindow->parentWidget()->hide();
}

void MainWindow::setBookmarksVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showBookmarksDockWidget();
    else
        hideBookmarksDockWidget();
}

void MainWindow::showBookmarksDockWidget()
{
    TRACE_OBJ
    activateDockWidget(m_bookmarkWidget);
}

void MainWindow::hideBookmarksDockWidget()
{
    TRACE_OBJ
    m_bookmarkWidget->parentWidget()->hide();
}

void MainWindow::setSearchVisible(bool visible)
{
    TRACE_OBJ
    if (visible)
        showSearch();
    else
        hideSearch();
}

void MainWindow::showSearch()
{
    TRACE_OBJ
    activateDockWidget(m_searchWindow);
}

void MainWindow::showOpenPages()
{
    TRACE_OBJ
    activateDockWidget(OpenPagesManager::instance()->openPagesWidget());
}

void MainWindow::hideSearch()
{
    TRACE_OBJ
    m_searchWindow->parentWidget()->hide();
}

void MainWindow::activateDockWidget(QWidget *w)
{
    TRACE_OBJ
    w->parentWidget()->show();
    w->parentWidget()->raise();
    w->setFocus();
}

void MainWindow::setIndexString(const QString &str)
{
    TRACE_OBJ
    m_indexWindow->setSearchLineEditText(str);
}

void MainWindow::activateCurrentBrowser()
{
    TRACE_OBJ
    CentralWidget::instance()->activateTab();
}

void MainWindow::activateCurrentCentralWidgetTab()
{
    TRACE_OBJ
    m_centralWidget->activateTab();
}

void MainWindow::updateApplicationFont()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    QFont font = qApp->font();
    if (helpEngine.usesAppFont())
        font = helpEngine.appFont();

    const QWidgetList &widgets = QApplication::allWidgets();
    for (QWidget *widget : widgets)
        widget->setFont(font);
}

void MainWindow::setupFilterCombo()
{
    TRACE_OBJ
    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    const QString currentFilter = helpEngine.filterEngine()->activeFilter();
    m_filterCombo->clear();
    m_filterCombo->addItem(tr("Unfiltered"));
    const QStringList allFilters = helpEngine.filterEngine()->filters();
    if (!allFilters.isEmpty())
        m_filterCombo->insertSeparator(1);
    for (const QString &filter : allFilters)
        m_filterCombo->addItem(filter, filter);

    int idx = m_filterCombo->findData(currentFilter);
    if (idx < 0)
        idx = 0;
    m_filterCombo->setCurrentIndex(idx);
}

void MainWindow::filterDocumentation(int filterIndex)
{
    TRACE_OBJ

    const QString filter = m_filterCombo->itemData(filterIndex).toString();
    HelpEngineWrapper::instance().filterEngine()->setActiveFilter(filter);
}

void MainWindow::expandTOC(int depth)
{
    TRACE_OBJ
    Q_ASSERT(depth >= -1);
    m_contentWindow->expandToDepth(depth);
}

void MainWindow::indexingStarted()
{
    TRACE_OBJ
    if (!m_progressWidget) {
        m_progressWidget = new QWidget();
        QLayout* hlayout = new QHBoxLayout(m_progressWidget);

        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

        QLabel *label = new QLabel(tr("Updating search index"));
        label->setSizePolicy(sizePolicy);
        hlayout->addWidget(label);

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 0);
        progressBar->setTextVisible(false);
        progressBar->setSizePolicy(sizePolicy);

        hlayout->setSpacing(6);
        hlayout->setContentsMargins(QMargins());
        hlayout->addWidget(progressBar);

        statusBar()->addPermanentWidget(m_progressWidget);
    }
}

void MainWindow::indexingFinished()
{
    TRACE_OBJ
    statusBar()->removeWidget(m_progressWidget);
    delete m_progressWidget;
    m_progressWidget = nullptr;
}

QString MainWindow::collectionFileDirectory(bool createDir, const QString &cacheDir)
{
    TRACE_OBJ
    QString collectionPath =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (collectionPath.isEmpty()) {
        if (cacheDir.isEmpty())
            collectionPath = QDir::homePath() + QDir::separator() + ".assistant"_L1;
        else
            collectionPath = QDir::homePath() + "/."_L1 + cacheDir;
    } else {
        if (cacheDir.isEmpty())
            collectionPath = collectionPath + "/QtProject/Assistant"_L1;
        else
            collectionPath = collectionPath + QDir::separator() + cacheDir;
    }
    if (createDir) {
        QDir dir;
        if (!dir.exists(collectionPath))
            dir.mkpath(collectionPath);
    }
    return collectionPath;
}

QString MainWindow::defaultHelpCollectionFileName()
{
    TRACE_OBJ
    // forces creation of the default collection file path
    return collectionFileDirectory(true) + QDir::separator()
            + QString("qthelpcollection_%1.qhc"_L1).arg(QLatin1StringView(QT_VERSION_STR));
}

void MainWindow::currentFilterChanged(const QString &filter)
{
    TRACE_OBJ
    int index = m_filterCombo->findData(filter);
    if (index < 0)
        index = 0;
    m_filterCombo->setCurrentIndex(index);
}

void MainWindow::documentationRemoved(const QString &namespaceName)
{
    TRACE_OBJ
    OpenPagesManager::instance()->closePages(namespaceName);
}

void MainWindow::documentationUpdated(const QString &namespaceName)
{
    TRACE_OBJ
    OpenPagesManager::instance()->reloadPages(namespaceName);
}

void MainWindow::resetQtDocInfo(const QString &component)
{
    TRACE_OBJ
    HelpEngineWrapper::instance().setQtDocInfo(component,
        QStringList(QDateTime().toString(Qt::ISODate)));
}

void MainWindow::registerDocumentation(const QString &component,
                                       const QString &absFileName)
{
    TRACE_OBJ
    QString ns = QHelpEngineCore::namespaceName(absFileName);
    if (ns.isEmpty())
        return;

    HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
    if (helpEngine.registeredDocumentations().contains(ns))
        helpEngine.unregisterDocumentation(ns);
    if (!helpEngine.registerDocumentation(absFileName)) {
        QMessageBox::warning(this, tr("Qt Assistant"),
            tr("Could not register file '%1': %2").
            arg(absFileName).arg(helpEngine.error()));
    } else {
        QStringList docInfo;
        docInfo << QFileInfo(absFileName).lastModified().toString(Qt::ISODate)
                << absFileName;
        helpEngine.setQtDocInfo(component, docInfo);
    }
}

void MainWindow::handlePageCountChanged()
{
    m_closeTabAction->setEnabled(OpenPagesManager::instance()->pageCount() > 1);
}

QT_END_NAMESPACE
