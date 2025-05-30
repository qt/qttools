// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef COLLECTIONCONFIGREADER_H
#define COLLECTIONCONFIGREADER_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QXmlStreamReader>

QT_USE_NAMESPACE

class CollectionConfigReader : public QXmlStreamReader
{
public:
    void readData(const QByteArray &contents);

    QString title() const { return m_title; }
    QString homePage() const { return m_homePage; }
    QString startPage() const { return m_startPage; }
    QString applicationIcon() const { return m_applicationIcon; }
    QString currentFilter() const { return m_currentFilter; }
    bool enableFilterFunctionality() const
        { return m_enableFilterFunctionality; }
    bool hideFilterFunctionality() const
        { return m_hideFilterFunctionality; }
    bool enableAddressBar() const { return m_enableAddressBar; }
    bool hideAddressBar() const { return m_hideAddressBar; }
    bool enableDocumentationManager() const
        { return m_enableDocumentationManager; }

    QMap<QString, QString> aboutMenuTexts() const
        { return m_aboutMenuTexts; }
    QString aboutIcon() const { return m_aboutIcon; }
    QMap<QString, QString> aboutTextFiles() const
        { return m_aboutTextFiles; }

    QMap<QString, QString> filesToGenerate() const
        { return m_filesToGenerate; }

    QStringList filesToRegister() const { return m_filesToRegister; }

    QString cacheDirectory() const { return m_cacheDirectory; }
    bool cacheDirRelativeToCollection() const { return m_cacheDirRelativeToCollection; }

    bool fullTextSearchFallbackEnabled() const {
        return m_enableFullTextSearchFallback;
    }

private:
    void raiseErrorWithLine();
    void readConfig();
    void readAssistantSettings();
    void readMenuTexts();
    void readAboutDialog();
    void readDocFiles();
    void readGenerate();
    void readFiles();
    void readRegister();

    QMap<QString, QString> m_aboutMenuTexts;
    QMap<QString, QString> m_aboutTextFiles;
    QMap<QString, QString> m_filesToGenerate;
    QStringList m_filesToRegister;
    QString m_title;
    QString m_homePage;
    QString m_startPage;
    QString m_applicationIcon;
    QString m_currentFilter;
    QString m_aboutIcon;
    QString m_cacheDirectory;
    bool m_enableFilterFunctionality;
    bool m_hideFilterFunctionality;
    bool m_enableAddressBar;
    bool m_hideAddressBar;
    bool m_enableDocumentationManager;
    bool m_cacheDirRelativeToCollection;
    bool m_enableFullTextSearchFallback;
};

#endif
