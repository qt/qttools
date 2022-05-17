// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore/QLibraryInfo>
#include <QtWidgets/QApplication>
#include <QtHelp/QHelpEngineCore>

#include "helpbrowser.h"
#include "qhelplink.h"

HelpBrowser::HelpBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    QString collectionFile = QLibraryInfo::path(QLibraryInfo::ExamplesPath)
        + QLatin1String("/help/contextsensitivehelp/docs/wateringmachine.qhc");

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

