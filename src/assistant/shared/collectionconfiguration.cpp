// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "collectionconfiguration.h"

#include <QtHelp/QHelpEngineCore>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {
    const QString AboutIconKey("AboutIcon"_L1);
    const QString AboutImagesKey("AboutImages"_L1);
    const QString AboutMenuTextsKey("AboutMenuTexts"_L1);
    const QString AboutTextsKey("AboutTexts"_L1);
    const QString ApplicationIconKey("ApplicationIcon"_L1);
    const QString CacheDirKey("CacheDirectory"_L1);
    const QString CacheDirRelativeToCollectionKey("CacheDirRelativeToCollection"_L1);
    const QString CreationTimeKey("CreationTime"_L1);
    const QString DefaultHomePageKey("defaultHomepage"_L1);
    const QString EnableAddressBarKey("EnableAddressBar"_L1);
    const QString EnableDocManagerKey("EnableDocumentationManager"_L1);
    const QString EnableFilterKey("EnableFilterFunctionality"_L1);
    const QString HideAddressBarKey("HideAddressBar"_L1);
    const QString FilterToolbarHiddenKey("HideFilterFunctionality"_L1);
    const QString LastPageKey("LastTabPage"_L1);
    const QString LastRegisterTime("LastRegisterTime"_L1);
    const QString LastShownPagesKey("LastShownPages"_L1);
    const QString LastZoomFactorsKey(
#if defined(BROWSER_QTWEBKIT)
            "LastPagesZoomWebView"_L1
#else
            "LastPagesZoomTextBrowser"_L1
#endif
            );
    const QString WindowTitleKey("WindowTitle"_L1);
    const QString FullTextSearchFallbackKey("FullTextSearchFallback"_L1);
} // anonymous namespace

const QString CollectionConfiguration::DefaultZoomFactor("0.0"_L1);
const QString CollectionConfiguration::ListSeparator("|"_L1);

uint CollectionConfiguration::creationTime(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(CreationTimeKey, 0).toUInt();
}

void CollectionConfiguration::setCreationTime(QHelpEngineCore &helpEngine, uint time)
{
    helpEngine.setCustomValue(CreationTimeKey, time);
}

const QString CollectionConfiguration::windowTitle(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(WindowTitleKey).toString();
}

void CollectionConfiguration::setWindowTitle(QHelpEngineCore &helpEngine,
                                             const QString &windowTitle)
{
    helpEngine.setCustomValue(WindowTitleKey, windowTitle);
}

bool CollectionConfiguration::filterFunctionalityEnabled(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(EnableFilterKey, true).toBool();
}

void CollectionConfiguration::setFilterFunctionalityEnabled(QHelpEngineCore &helpEngine,
                                                            bool enabled)
{
    helpEngine.setCustomValue(EnableFilterKey, enabled);
}

bool CollectionConfiguration::filterToolbarVisible(const QHelpEngineCore &helpEngine)
{
    return !helpEngine.customValue(FilterToolbarHiddenKey, true).toBool();
}

void CollectionConfiguration::setFilterToolbarVisible(QHelpEngineCore &helpEngine,
                                                      bool visible)
{
    helpEngine.setCustomValue(FilterToolbarHiddenKey, !visible);
}

bool CollectionConfiguration::addressBarEnabled(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(EnableAddressBarKey, true).toBool();
}

void CollectionConfiguration::setAddressBarEnabled(QHelpEngineCore &helpEngine,
                                                   bool enabled)
{
    helpEngine.setCustomValue(EnableAddressBarKey, enabled);
}

bool CollectionConfiguration::addressBarVisible(const QHelpEngineCore &helpEngine)
{
    return !helpEngine.customValue(HideAddressBarKey, true).toBool();
}

void CollectionConfiguration::setAddressBarVisible(QHelpEngineCore &helpEngine,
                                                   bool visible)
{
    helpEngine.setCustomValue(HideAddressBarKey, !visible);
}

const QString CollectionConfiguration::cacheDir(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(CacheDirKey).toString();
}

bool CollectionConfiguration::cacheDirIsRelativeToCollection(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(CacheDirRelativeToCollectionKey).toBool();
}

void CollectionConfiguration::setCacheDir(QHelpEngineCore &helpEngine,
                            const QString &cacheDir, bool relativeToCollection)
{
    helpEngine.setCustomValue(CacheDirKey, cacheDir);
    helpEngine.setCustomValue(CacheDirRelativeToCollectionKey,
                              relativeToCollection);
}

bool CollectionConfiguration::documentationManagerEnabled(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(EnableDocManagerKey, true).toBool();
}

void CollectionConfiguration::setDocumentationManagerEnabled(QHelpEngineCore &helpEngine,
                                                             bool enabled)
{
    helpEngine.setCustomValue(EnableDocManagerKey, enabled);
}

const QByteArray CollectionConfiguration::applicationIcon(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(ApplicationIconKey).toByteArray();
}

void CollectionConfiguration::setApplicationIcon(QHelpEngineCore &helpEngine,
                                                 const QByteArray &icon)
{
    helpEngine.setCustomValue(ApplicationIconKey, icon);
}

const QByteArray CollectionConfiguration::aboutMenuTexts(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(AboutMenuTextsKey).toByteArray();
}

void CollectionConfiguration::setAboutMenuTexts(QHelpEngineCore &helpEngine,
                                                const QByteArray &texts)
{
    helpEngine.setCustomValue(AboutMenuTextsKey, texts);
}

const QByteArray CollectionConfiguration::aboutIcon(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(AboutIconKey).toByteArray();
}

void CollectionConfiguration::setAboutIcon(QHelpEngineCore &helpEngine,
                                           const QByteArray &icon)
{
    helpEngine.setCustomValue(AboutIconKey, icon);
}

const QByteArray CollectionConfiguration::aboutTexts(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(AboutTextsKey).toByteArray();
}

void CollectionConfiguration::setAboutTexts(QHelpEngineCore &helpEngine,
                                            const QByteArray &texts)
{
    helpEngine.setCustomValue(AboutTextsKey, texts);
}

const QByteArray CollectionConfiguration::aboutImages(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(AboutImagesKey).toByteArray();
}

void CollectionConfiguration::setAboutImages(QHelpEngineCore &helpEngine,
                                             const QByteArray &images)
{
    helpEngine.setCustomValue(AboutImagesKey, images);
}

const QString CollectionConfiguration::defaultHomePage(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(DefaultHomePageKey, "help"_L1).toString();
}

void CollectionConfiguration::setDefaultHomePage(QHelpEngineCore &helpEngine,
                                                 const QString &page)
{
    helpEngine.setCustomValue(DefaultHomePageKey, page);
}

const QStringList CollectionConfiguration::lastShownPages(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(LastShownPagesKey).toString().
        split(ListSeparator, Qt::SkipEmptyParts);
}

void CollectionConfiguration::setLastShownPages(QHelpEngineCore &helpEngine,
                                             const QStringList &lastShownPages)
{
    helpEngine.setCustomValue(LastShownPagesKey,
                              lastShownPages.join(ListSeparator));
}

const QStringList CollectionConfiguration::lastZoomFactors(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(LastZoomFactorsKey).toString().
        split(ListSeparator, Qt::SkipEmptyParts);
}

void CollectionConfiguration::setLastZoomFactors(QHelpEngineCore &helpEngine,
                                             const QStringList &lastZoomFactors)
{
    helpEngine.setCustomValue(LastZoomFactorsKey,
                              lastZoomFactors.join(ListSeparator));
}

int CollectionConfiguration::lastTabPage(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(LastPageKey, 1).toInt();
}

void CollectionConfiguration::setLastTabPage(QHelpEngineCore &helpEngine,
                                             int lastPage)
{
    helpEngine.setCustomValue(LastPageKey, lastPage);
}

const QDateTime CollectionConfiguration::lastRegisterTime(const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(LastRegisterTime, QDateTime()).toDateTime();
}

void CollectionConfiguration::updateLastRegisterTime(QHelpEngineCore &helpEngine, QDateTime dt)
{
    helpEngine.setCustomValue(LastRegisterTime, dt);
}

void CollectionConfiguration::updateLastRegisterTime(QHelpEngineCore &helpEngine)
{
    updateLastRegisterTime(helpEngine, QDateTime::currentDateTime());
}

bool CollectionConfiguration::isNewer(const QHelpEngineCore &newer,
                                const QHelpEngineCore &older)
{
    return creationTime(newer) > creationTime(older);
}

void CollectionConfiguration::copyConfiguration(const QHelpEngineCore &source,
                                                QHelpEngineCore &target)
{
    setCreationTime(target, creationTime(source));
    setWindowTitle(target, windowTitle(source));
    target.setCurrentFilter(source.currentFilter());
    setCacheDir(target, cacheDir(source), cacheDirIsRelativeToCollection(source));
    setFilterFunctionalityEnabled(target, filterFunctionalityEnabled(source));
    setFilterToolbarVisible(target, filterToolbarVisible(source));
    setAddressBarEnabled(target, addressBarEnabled(source));
    setAddressBarVisible(target, addressBarVisible(source));
    setDocumentationManagerEnabled(target, documentationManagerEnabled(source));
    setApplicationIcon(target, applicationIcon(source));
    setAboutMenuTexts(target, aboutMenuTexts(source));
    setAboutIcon(target, aboutIcon(source));
    setAboutTexts(target, aboutTexts(source));
    setAboutImages(target, aboutImages(source));
    setDefaultHomePage(target, defaultHomePage(source));
    setFullTextSearchFallbackEnabled(target, fullTextSearchFallbackEnabled(source));
}

bool CollectionConfiguration:: fullTextSearchFallbackEnabled(
    const QHelpEngineCore &helpEngine)
{
    return helpEngine.customValue(FullTextSearchFallbackKey, false).toBool();
}

void CollectionConfiguration::setFullTextSearchFallbackEnabled(
    QHelpEngineCore &helpEngine, bool on)
{
    helpEngine.setCustomValue(FullTextSearchFallbackKey, on);
}

QT_END_NAMESPACE
