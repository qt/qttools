// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpengineplugin.h"

#include <algorithm>
#include <iterator>
#include <memory>

QT_BEGIN_NAMESPACE

static std::vector<QQmlLSHelpProvider::DocumentLink>
transformQHelpLink(QList<QHelpLink> &&qhelplinklist)
{
    std::vector<QQmlLSHelpProvider::DocumentLink> result(qhelplinklist.size());
    std::transform(qhelplinklist.begin(), qhelplinklist.end(), result.begin(),
                   [&](const auto &item) {
                       QQmlLSHelpProvider::DocumentLink element;
                       element.title = item.title;
                       element.url = item.url;
                       return element;
                   });
    return result;
}

QQmlLSHelpProvider::QQmlLSHelpProvider(const QString &qhcFilePath, QObject *parent)
{
    m_helpEngine.emplace(qhcFilePath, parent);
    m_helpEngine->setReadOnly(false);
    m_helpEngine->setupData();
}

bool QQmlLSHelpProvider::registerDocumentation(const QString &documentationFileName)
{
    Q_ASSERT(m_helpEngine.has_value());
    return m_helpEngine->registerDocumentation(documentationFileName);
}

QByteArray QQmlLSHelpProvider::fileData(const QUrl &url) const
{
    Q_ASSERT(m_helpEngine.has_value());
    return m_helpEngine->fileData(url);
}

std::vector<QQmlLSHelpProvider::DocumentLink>
QQmlLSHelpProvider::documentsForIdentifier(const QString &id) const
{
    Q_ASSERT(m_helpEngine.has_value());
    return transformQHelpLink(m_helpEngine->documentsForIdentifier(id));
}

std::vector<QQmlLSHelpProvider::DocumentLink>
QQmlLSHelpProvider::documentsForIdentifier(const QString &id, const QString &filterName) const
{
    Q_ASSERT(m_helpEngine.has_value());
    return transformQHelpLink(m_helpEngine->documentsForIdentifier(id, filterName));
}

std::vector<QQmlLSHelpProvider::DocumentLink>
QQmlLSHelpProvider::documentsForKeyword(const QString &keyword) const
{
    Q_ASSERT(m_helpEngine.has_value());
    return transformQHelpLink(m_helpEngine->documentsForKeyword(keyword));
}

std::vector<QQmlLSHelpProvider::DocumentLink>
QQmlLSHelpProvider::documentsForKeyword(const QString &keyword, const QString &filter) const
{
    Q_ASSERT(m_helpEngine.has_value());
    return transformQHelpLink(m_helpEngine->documentsForKeyword(keyword, filter));
}

QStringList QQmlLSHelpProvider::registeredNamespaces() const
{
    Q_ASSERT(m_helpEngine.has_value());
    return m_helpEngine->registeredDocumentations();
}

QString QQmlLSHelpProvider::error() const
{
    Q_ASSERT(m_helpEngine.has_value());
    return m_helpEngine->error();
}

QHelpEnginePlugin::QHelpEnginePlugin(QObject *parent) : QObject(parent) { }

std::unique_ptr<QQmlLSHelpProviderBase> QHelpEnginePlugin::initialize(const QString &collectionFile,
                                                                      QObject *parent)
{
    return std::make_unique<QQmlLSHelpProvider>(collectionFile, parent);
}

QT_END_NAMESPACE

#include "moc_qhelpengineplugin.cpp"
