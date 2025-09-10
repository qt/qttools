// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "tracer.h"

#include "helpenginewrapper.h"
#include "../shared/collectionconfiguration.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QTimer>
#include <QtHelp/QHelpContentModel>
#include <QtHelp/QHelpEngine>
#include <QtHelp/QHelpFilterEngine>
#include <QtHelp/QHelpIndexModel>
#include <QtHelp/QHelpLink>
#include <QtHelp/QHelpSearchEngine>

#include <map>
#include <memory>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {
    const QString AppFontKey("appFont"_L1);
    const QString AppWritingSystemKey("appWritingSystem"_L1);
    const QString BookmarksKey("Bookmarks"_L1);
    const QString BrowserFontKey("browserFont"_L1);
    const QString BrowserWritingSystemKey("browserWritingSystem"_L1);
    const QString HomePageKey("homepage"_L1);
    const QString MainWindowKey("MainWindow"_L1);
    const QString MainWindowGeometryKey("MainWindowGeometry"_L1);
    const QString SearchWasAttachedKey("SearchWasAttached"_L1);
    const QString StartOptionKey("StartOption"_L1);
    const QString UseAppFontKey("useAppFont"_L1);
    const QString UseBrowserFontKey("useBrowserFont"_L1);
    const QString VersionKey("qtVersion%1$$$%2"_L1.arg(QLatin1StringView(QT_VERSION_STR)));
    const QString ShowTabsKey("showTabs"_L1);
    const QString TopicChooserGeometryKey("TopicChooserGeometry"_L1);
} // anonymous namespace

class TimeoutForwarder : public QObject
{
    Q_OBJECT
public:
    TimeoutForwarder(const QString &fileName);
private slots:
    void forward();
private:
    friend class HelpEngineWrapperPrivate;

    const QString m_fileName;
};

class HelpEngineWrapperPrivate : public QObject
{
    Q_OBJECT
    friend class HelpEngineWrapper;
    friend class TimeoutForwarder;
private slots:
    void qchFileChanged(const QString &fileName);

signals:
    void documentationRemoved(const QString &namespaceName);
    void documentationUpdated(const QString &namespaceName);

private:
    HelpEngineWrapperPrivate(const QString &collectionFile);

    void initFileSystemWatchers();
    void checkDocFilesWatched();
    void qchFileChanged(const QString &fileName, bool fromTimeout);

    static const int UpdateGracePeriod = 2000;

    QHelpEngine * const m_helpEngine;
    QFileSystemWatcher * const m_qchWatcher;
    struct RecentSignal {
        QDateTime timestamp;
        std::unique_ptr<TimeoutForwarder> forwarder;
    };
    std::map<QString, RecentSignal> m_recentQchUpdates;
};

HelpEngineWrapper *HelpEngineWrapper::helpEngineWrapper = nullptr;

HelpEngineWrapper &HelpEngineWrapper::instance()
{
    return instance({});
}

HelpEngineWrapper &HelpEngineWrapper::instance(const QString &collectionFile)
{
    TRACE_OBJ
    /*
     * Note that this Singleton cannot be static, because it has to be
     * deleted before the QApplication.
     */
    if (!helpEngineWrapper)
        helpEngineWrapper = new HelpEngineWrapper(collectionFile);
    return *helpEngineWrapper;
}

void HelpEngineWrapper::removeInstance()
{
    TRACE_OBJ
    delete helpEngineWrapper;
    helpEngineWrapper = nullptr;
}

HelpEngineWrapper::HelpEngineWrapper(const QString &collectionFile)
    : d(new HelpEngineWrapperPrivate(collectionFile))
{
    TRACE_OBJ

    /*
     * Otherwise we will waste time if several new docs are found,
     * because we will start to index them, only to be interrupted
     * by the next request. Also, there is a nasty SQLITE bug that will
     * cause the application to hang for minutes in that case.
     * This call is reverted by initialDocSetupDone(), which must be
     * called after the new docs have been installed.
     */
//  TODO: probably remove it
    disconnect(d->m_helpEngine, &QHelpEngineCore::setupFinished,
            searchEngine(), &QHelpSearchEngine::scheduleIndexDocumentation);

    connect(d, &HelpEngineWrapperPrivate::documentationRemoved,
            this, &HelpEngineWrapper::documentationRemoved);
    connect(d, &HelpEngineWrapperPrivate::documentationUpdated,
            this, &HelpEngineWrapper::documentationUpdated);
    connect(d->m_helpEngine, &QHelpEngineCore::setupFinished,
            this, &HelpEngineWrapper::setupFinished);
}

HelpEngineWrapper::~HelpEngineWrapper()
{
    TRACE_OBJ
    const QStringList &namespaces = d->m_helpEngine->registeredDocumentations();
    for (const QString &nameSpace : namespaces) {
        const QString &docFile
            = d->m_helpEngine->documentationFileName(nameSpace);
        d->m_qchWatcher->removePath(docFile);
    }

    delete d;
}

void HelpEngineWrapper::initialDocSetupDone()
{
    TRACE_OBJ
//  TODO: probably remove it
    connect(d->m_helpEngine, &QHelpEngineCore::setupFinished,
            searchEngine(), &QHelpSearchEngine::scheduleIndexDocumentation);
    setupData();
}

QHelpSearchEngine *HelpEngineWrapper::searchEngine() const
{
    TRACE_OBJ
    return d->m_helpEngine->searchEngine();
}

QHelpContentModel *HelpEngineWrapper::contentModel() const
{
    TRACE_OBJ
    return d->m_helpEngine->contentModel();
}

QHelpIndexModel *HelpEngineWrapper::indexModel() const
{
    TRACE_OBJ
    return d->m_helpEngine->indexModel();
}

QHelpContentWidget *HelpEngineWrapper::contentWidget()
{
    TRACE_OBJ
    return d->m_helpEngine->contentWidget();
}

QHelpIndexWidget *HelpEngineWrapper::indexWidget()
{
    TRACE_OBJ
    return d->m_helpEngine->indexWidget();
}

const QStringList HelpEngineWrapper::registeredDocumentations() const
{
    TRACE_OBJ
    return d->m_helpEngine->registeredDocumentations();
}

QString HelpEngineWrapper::documentationFileName(const QString &namespaceName) const
{
    TRACE_OBJ
    return d->m_helpEngine->documentationFileName(namespaceName);
}

const QString HelpEngineWrapper::collectionFile() const
{
    TRACE_OBJ
    return d->m_helpEngine->collectionFile();
}

bool HelpEngineWrapper::registerDocumentation(const QString &docFile)
{
    TRACE_OBJ
    d->checkDocFilesWatched();
    if (!d->m_helpEngine->registerDocumentation(docFile))
        return false;
    d->m_qchWatcher->addPath(docFile);
    d->checkDocFilesWatched();
    return true;
}

bool HelpEngineWrapper::unregisterDocumentation(const QString &namespaceName)
{
    TRACE_OBJ
    d->checkDocFilesWatched();
    const QString &file = d->m_helpEngine->documentationFileName(namespaceName);
    if (!d->m_helpEngine->unregisterDocumentation(namespaceName))
        return false;
    d->m_qchWatcher->removePath(file);
    d->checkDocFilesWatched();
    return true;
}

bool HelpEngineWrapper::setupData()
{
    TRACE_OBJ
    return d->m_helpEngine->setupData();
}

QUrl HelpEngineWrapper::findFile(const QUrl &url) const
{
    TRACE_OBJ
    return d->m_helpEngine->findFile(url);
}

QByteArray HelpEngineWrapper::fileData(const QUrl &url) const
{
    TRACE_OBJ
    return d->m_helpEngine->fileData(url);
}

QList<QHelpLink> HelpEngineWrapper::documentsForIdentifier(const QString &id) const
{
    TRACE_OBJ
    return d->m_helpEngine->documentsForIdentifier(id);
}

QString HelpEngineWrapper::error() const
{
    TRACE_OBJ
    return d->m_helpEngine->error();
}

QHelpFilterEngine *HelpEngineWrapper::filterEngine() const
{
    return d->m_helpEngine->filterEngine();
}

const QStringList HelpEngineWrapper::qtDocInfo(const QString &component) const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(VersionKey.arg(component)).toString().
        split(CollectionConfiguration::ListSeparator);
}

void HelpEngineWrapper::setQtDocInfo(const QString &component,
                                     const QStringList &doc)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(VersionKey.arg(component),
                              doc.join(CollectionConfiguration::ListSeparator));
}

const QStringList HelpEngineWrapper::lastShownPages() const
{
    TRACE_OBJ
    return CollectionConfiguration::lastShownPages(*d->m_helpEngine);
}

void HelpEngineWrapper::setLastShownPages(const QStringList &lastShownPages)
{
    TRACE_OBJ
    CollectionConfiguration::setLastShownPages(*d->m_helpEngine, lastShownPages);
}

const QStringList HelpEngineWrapper::lastZoomFactors() const
{
    TRACE_OBJ
    return CollectionConfiguration::lastZoomFactors(*d->m_helpEngine);
}

void HelpEngineWrapper::setLastZoomFactors(const QStringList &lastZoomFactors)
{
    TRACE_OBJ
    CollectionConfiguration::setLastZoomFactors(*d->m_helpEngine, lastZoomFactors);
}

const QString HelpEngineWrapper::cacheDir() const
{
    TRACE_OBJ
    return CollectionConfiguration::cacheDir(*d->m_helpEngine);
}

bool HelpEngineWrapper::cacheDirIsRelativeToCollection() const
{
    TRACE_OBJ
    return CollectionConfiguration::cacheDirIsRelativeToCollection(*d->m_helpEngine);
}

void HelpEngineWrapper::setCacheDir(const QString &cacheDir,
                                    bool relativeToCollection)
{
    TRACE_OBJ
    CollectionConfiguration::setCacheDir(*d->m_helpEngine, cacheDir,
                                         relativeToCollection);
}

bool HelpEngineWrapper::filterFunctionalityEnabled() const
{
    TRACE_OBJ
    return CollectionConfiguration::filterFunctionalityEnabled(*d->m_helpEngine);
}

void HelpEngineWrapper::setFilterFunctionalityEnabled(bool enabled)
{
    TRACE_OBJ
    CollectionConfiguration::setFilterFunctionalityEnabled(*d->m_helpEngine,
                                                           enabled);
}

bool HelpEngineWrapper::filterToolbarVisible() const
{
    TRACE_OBJ
    return CollectionConfiguration::filterToolbarVisible(*d->m_helpEngine);
}

void HelpEngineWrapper::setFilterToolbarVisible(bool visible)
{
    TRACE_OBJ
    CollectionConfiguration::setFilterToolbarVisible(*d->m_helpEngine, visible);
}

bool HelpEngineWrapper::addressBarEnabled() const
{
    TRACE_OBJ
    return CollectionConfiguration::addressBarEnabled(*d->m_helpEngine);
}

void HelpEngineWrapper::setAddressBarEnabled(bool enabled)
{
    TRACE_OBJ
    CollectionConfiguration::setAddressBarEnabled(*d->m_helpEngine, enabled);
}

bool HelpEngineWrapper::addressBarVisible() const
{
    TRACE_OBJ
    return CollectionConfiguration::addressBarVisible(*d->m_helpEngine);
}

void HelpEngineWrapper::setAddressBarVisible(bool visible)
{
    TRACE_OBJ
    CollectionConfiguration::setAddressBarVisible(*d->m_helpEngine, visible);
}

bool HelpEngineWrapper::documentationManagerEnabled() const
{
    TRACE_OBJ
    return CollectionConfiguration::documentationManagerEnabled(*d->m_helpEngine);
}

void HelpEngineWrapper::setDocumentationManagerEnabled(bool enabled)
{
    TRACE_OBJ
    CollectionConfiguration::setDocumentationManagerEnabled(*d->m_helpEngine,
                                                            enabled);
}

const QByteArray HelpEngineWrapper::aboutMenuTexts() const
{
    TRACE_OBJ
    return CollectionConfiguration::aboutMenuTexts(*d->m_helpEngine);
}

void HelpEngineWrapper::setAboutMenuTexts(const QByteArray &texts)
{
    TRACE_OBJ
    CollectionConfiguration::setAboutMenuTexts(*d->m_helpEngine, texts);
}

const QByteArray HelpEngineWrapper::aboutIcon() const
{
    TRACE_OBJ
    return CollectionConfiguration::aboutIcon(*d->m_helpEngine);
}

void HelpEngineWrapper::setAboutIcon(const QByteArray &icon)
{
    TRACE_OBJ
    CollectionConfiguration::setAboutIcon(*d->m_helpEngine, icon);
}

const QByteArray HelpEngineWrapper::aboutImages() const
{
    TRACE_OBJ
    return CollectionConfiguration::aboutImages(*d->m_helpEngine);
}

void HelpEngineWrapper::setAboutImages(const QByteArray &images)
{
    TRACE_OBJ
    CollectionConfiguration::setAboutImages(*d->m_helpEngine, images);
}

const QByteArray HelpEngineWrapper::aboutTexts() const
{
    TRACE_OBJ
    return CollectionConfiguration::aboutTexts(*d->m_helpEngine);
}

void HelpEngineWrapper::setAboutTexts(const QByteArray &texts)
{
    TRACE_OBJ
    CollectionConfiguration::setAboutTexts(*d->m_helpEngine, texts);
}

const QString HelpEngineWrapper::windowTitle() const
{
    TRACE_OBJ
    return CollectionConfiguration::windowTitle(*d->m_helpEngine);
}

void HelpEngineWrapper::setWindowTitle(const QString &windowTitle)
{
    TRACE_OBJ
    CollectionConfiguration::setWindowTitle(*d->m_helpEngine, windowTitle);
}

const QByteArray HelpEngineWrapper::applicationIcon() const
{
    TRACE_OBJ
    return CollectionConfiguration::applicationIcon(*d->m_helpEngine);
}

void HelpEngineWrapper::setApplicationIcon(const QByteArray &icon)
{
    TRACE_OBJ
    CollectionConfiguration::setApplicationIcon(*d->m_helpEngine, icon);
}

const QByteArray HelpEngineWrapper::mainWindow() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(MainWindowKey).toByteArray();
}

void HelpEngineWrapper::setMainWindow(const QByteArray &mainWindow)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(MainWindowKey, mainWindow);
}

const QByteArray HelpEngineWrapper::mainWindowGeometry() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(MainWindowGeometryKey).toByteArray();
}

void HelpEngineWrapper::setMainWindowGeometry(const QByteArray &geometry)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(MainWindowGeometryKey, geometry);
}

const QByteArray HelpEngineWrapper::bookmarks() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(BookmarksKey).toByteArray();
}

void HelpEngineWrapper::setBookmarks(const QByteArray &bookmarks)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(BookmarksKey, bookmarks);
}

int HelpEngineWrapper::lastTabPage() const
{
    TRACE_OBJ
    return CollectionConfiguration::lastTabPage(*d->m_helpEngine);
}

void HelpEngineWrapper::setLastTabPage(int lastPage)
{
    TRACE_OBJ
    CollectionConfiguration::setLastTabPage(*d->m_helpEngine, lastPage);
}

int HelpEngineWrapper::startOption() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(StartOptionKey, ShowLastPages).toInt();
}

void HelpEngineWrapper::setStartOption(int option)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(StartOptionKey, option);
}

const QString HelpEngineWrapper::homePage() const
{
    TRACE_OBJ
    const QString &homePage
        = d->m_helpEngine->customValue(HomePageKey).toString();
    if (!homePage.isEmpty())
        return homePage;
    return defaultHomePage();
}

void HelpEngineWrapper::setHomePage(const QString &page)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(HomePageKey, page);

}

const QString HelpEngineWrapper::defaultHomePage() const
{
    TRACE_OBJ
    return CollectionConfiguration::defaultHomePage(*d->m_helpEngine);
}

void HelpEngineWrapper::setDefaultHomePage(const QString &page)
{
    TRACE_OBJ
    CollectionConfiguration::setDefaultHomePage(*d->m_helpEngine, page);
}

bool HelpEngineWrapper::hasFontSettings() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(UseAppFontKey).isValid();
}

bool HelpEngineWrapper::usesAppFont() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(UseAppFontKey).toBool();
}

void HelpEngineWrapper::setUseAppFont(bool useAppFont)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(UseAppFontKey, useAppFont);
}

bool HelpEngineWrapper::usesBrowserFont() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(UseBrowserFontKey, false).toBool();
}

void HelpEngineWrapper::setUseBrowserFont(bool useBrowserFont)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(UseBrowserFontKey, useBrowserFont);
}

const QFont HelpEngineWrapper::appFont() const
{
    TRACE_OBJ
    return qvariant_cast<QFont>(d->m_helpEngine->customValue(AppFontKey));
}

void HelpEngineWrapper::setAppFont(const QFont &font)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(AppFontKey, font);
}

QFontDatabase::WritingSystem HelpEngineWrapper::appWritingSystem() const
{
    TRACE_OBJ
    return static_cast<QFontDatabase::WritingSystem>(
        d->m_helpEngine->customValue(AppWritingSystemKey).toInt());
}

void HelpEngineWrapper::setAppWritingSystem(QFontDatabase::WritingSystem system)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(AppWritingSystemKey, system);
}

const QFont HelpEngineWrapper::browserFont() const
{
    TRACE_OBJ
    return qvariant_cast<QFont>(d->m_helpEngine->customValue(BrowserFontKey));
}

void HelpEngineWrapper::setBrowserFont(const QFont &font)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(BrowserFontKey, font);
}

QFontDatabase::WritingSystem HelpEngineWrapper::browserWritingSystem() const
{
    TRACE_OBJ
    return static_cast<QFontDatabase::WritingSystem>(
        d->m_helpEngine->customValue(BrowserWritingSystemKey).toInt());
}

void HelpEngineWrapper::setBrowserWritingSystem(QFontDatabase::WritingSystem system)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(BrowserWritingSystemKey, system);
}

bool HelpEngineWrapper::showTabs() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(ShowTabsKey, false).toBool();
}

void HelpEngineWrapper::setShowTabs(bool show)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(ShowTabsKey, show);
}

bool HelpEngineWrapper::fullTextSearchFallbackEnabled() const
{
    TRACE_OBJ
    return CollectionConfiguration::fullTextSearchFallbackEnabled(*d->m_helpEngine);
}

const QByteArray HelpEngineWrapper::topicChooserGeometry() const
{
    TRACE_OBJ
    return d->m_helpEngine->customValue(TopicChooserGeometryKey).toByteArray();
}

void HelpEngineWrapper::setTopicChooserGeometry(const QByteArray &geometry)
{
    TRACE_OBJ
    d->m_helpEngine->setCustomValue(TopicChooserGeometryKey, geometry);
}

QHelpEngineCore *HelpEngineWrapper::helpEngine() const
{
    return d->m_helpEngine;
}


// -- TimeoutForwarder

TimeoutForwarder::TimeoutForwarder(const QString &fileName)
    : m_fileName(fileName)
{
    TRACE_OBJ
}

void TimeoutForwarder::forward()
{
    TRACE_OBJ
    HelpEngineWrapper::instance().d->qchFileChanged(m_fileName, true);
}

// -- HelpEngineWrapperPrivate

HelpEngineWrapperPrivate::HelpEngineWrapperPrivate(const QString &collectionFile)
    : m_helpEngine(new QHelpEngine(collectionFile, this)),
      m_qchWatcher(new QFileSystemWatcher(this))
{
    TRACE_OBJ
    m_helpEngine->setReadOnly(false);
    m_helpEngine->setUsesFilterEngine(true);
    initFileSystemWatchers();
}

void HelpEngineWrapperPrivate::initFileSystemWatchers()
{
    TRACE_OBJ
    for (const QString &ns : m_helpEngine->registeredDocumentations())
        m_qchWatcher->addPath(m_helpEngine->documentationFileName(ns));

    connect(m_qchWatcher, &QFileSystemWatcher::fileChanged, this,
            QOverload<const QString &>::of(&HelpEngineWrapperPrivate::qchFileChanged));
    checkDocFilesWatched();
}

void HelpEngineWrapperPrivate::qchFileChanged(const QString &fileName)
{
    TRACE_OBJ
    qchFileChanged(fileName, false);
}

void HelpEngineWrapperPrivate::checkDocFilesWatched()
{
    TRACE_OBJ
    const int watchedFilesCount = m_qchWatcher->files().size();
    const int docFilesCount = m_helpEngine->registeredDocumentations().size();
    if (watchedFilesCount != docFilesCount) {
        qWarning("Strange: Have %d docs, but %d are being watched",
                 watchedFilesCount, docFilesCount);
    }
}

void HelpEngineWrapperPrivate::qchFileChanged(const QString &fileName,
                                              bool fromTimeout)
{
    TRACE_OBJ

    /*
     * We don't use QHelpEngineCore::namespaceName(fileName), because the file
     * may not exist anymore or contain a different namespace.
     */
    QString ns;
    for (const QString &curNs : m_helpEngine->registeredDocumentations()) {
        if (m_helpEngine->documentationFileName(curNs) == fileName) {
            ns = curNs;
            break;
        }
    }

    /*
     * We can't do an assertion here, because QFileSystemWatcher may send the
     * signal more than  once.
     */
    if (ns.isEmpty()) {
        m_recentQchUpdates.erase(fileName);
        return;
    }

    /*
     * Since the QFileSystemWatcher typically sends the signal more than once,
     * we repeatedly delay our reaction a bit until we think the last signal
     * was sent.
     */

    const auto &it = m_recentQchUpdates.find(fileName);
    const QDateTime now = QDateTime::currentDateTimeUtc();

     // Case 1: This is the first recent signal for the file.
    if (it == m_recentQchUpdates.end()) {
        auto forwarder = std::make_unique<TimeoutForwarder>(fileName);
        QTimer::singleShot(UpdateGracePeriod, forwarder.get(),
                           &TimeoutForwarder::forward);
        m_recentQchUpdates.emplace(fileName,
                                   RecentSignal{std::move(now), std::move(forwarder)});
        return;
    }

    auto &[key, entry] = *it;

    // Case 2: The last signal for this file has not expired yet.
    if (entry.timestamp > now.addMSecs(-UpdateGracePeriod)) {
        if (!fromTimeout)
            entry.timestamp = now;
        else
            QTimer::singleShot(UpdateGracePeriod, entry.forwarder.get(),
                               &TimeoutForwarder::forward);
        return;
    }

    // Case 3: The last signal for this file has expired.
    if (m_helpEngine->unregisterDocumentation(ns)) {
        if (!QFileInfo(fileName).exists()
            || !m_helpEngine->registerDocumentation(fileName)) {
            m_qchWatcher->removePath(fileName);
            emit documentationRemoved(ns);
        } else {
            emit documentationUpdated(ns);
        }
        m_helpEngine->setupData();
    }
    m_recentQchUpdates.erase(it);
}

QT_END_NAMESPACE

#include "helpenginewrapper.moc"
