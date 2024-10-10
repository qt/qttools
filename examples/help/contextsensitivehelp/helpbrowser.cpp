// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QFileInfo>
#include <QtCore/QLibraryInfo>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>
#include <QtHelp/QHelpEngineCore>

#include "helpbrowser.h"
#include "qhelplink.h"

using namespace Qt::StringLiterals;

static QString documentationDirectory()
{
    QStringList paths;
#ifdef SRCDIR
    paths.append(QLatin1StringView(SRCDIR));
#endif
    paths.append(QLibraryInfo::path(QLibraryInfo::ExamplesPath));
    paths.append(QCoreApplication::applicationDirPath());
    paths.append(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation));
    for (const auto &dir : std::as_const(paths)) {
        const QString path = dir + "/docs"_L1;
        if (QFileInfo::exists(path))
            return path;
    }
    return {};
}

HelpBrowser::HelpBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    const QString collectionFile = documentationDirectory() + "/wateringmachine.qhc"_L1;

    m_helpEngine = new QHelpEngineCore(collectionFile, this);
    if (!m_helpEngine->setupData()) {
        delete m_helpEngine;
        m_helpEngine = nullptr;
    }
}

void HelpBrowser::showHelpForKeyword(const QString &id)
{
    if (m_helpEngine) {
        QList<QHelpLink> documents = m_helpEngine->documentsForIdentifier(id);
        if (documents.count())
            setSource(documents.first().url);
    }
}

QVariant HelpBrowser::loadResource(int type, const QUrl &name)
{
    QByteArray ba;
    if (type < 4 && m_helpEngine) {
        QUrl url(name);
        if (name.isRelative())
            url = source().resolved(url);
        ba = m_helpEngine->fileData(url);
    }
    return ba;
}

