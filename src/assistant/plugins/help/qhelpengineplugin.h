// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELPENGINEPLUGIN_H
#define QHELPENGINEPLUGIN_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtCore/qplugin.h>
#include <QtHelp/qhelpenginecore.h>
#include <QtHelp/qhelplink.h>

#include <QtQmlLS/private/qqmllshelpplugininterface_p.h>

#include <optional>
#include <vector>

QT_BEGIN_NAMESPACE

// TODO (Qt 7.0)
// Remove this plugin from QtTools when the QtHelp lib is split into
// QtHelpCore and QtHelp. Then QmlLS can depend only on QtHelpCore.
class QQmlLSHelpProvider : public QQmlLSHelpProviderBase
{
public:
    QQmlLSHelpProvider(const QString &qhcFile, QObject *parent = nullptr);
    bool registerDocumentation(const QString &documentationFileName) override;
    [[nodiscard]] QByteArray fileData(const QUrl &url) const override;
    [[nodiscard]] std::vector<DocumentLink> documentsForIdentifier(const QString &id) const override;
    [[nodiscard]] std::vector<DocumentLink> documentsForIdentifier(const QString &id, const QString &filterName) const override;
    [[nodiscard]] std::vector<DocumentLink> documentsForKeyword(const QString &keyword) const override;
    [[nodiscard]] std::vector<DocumentLink> documentsForKeyword(const QString &keyword, const QString &filterName) const override;
    [[nodiscard]] QStringList registeredNamespaces() const override;
    [[nodiscard]] QString error() const override;

private:
    std::optional<QHelpEngineCore> m_helpEngine;
};

class QHelpEnginePlugin : public QObject, public QQmlLSHelpPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlLSHelpPluginInterface_iid)
    Q_INTERFACES(QQmlLSHelpPluginInterface)
public:
    QHelpEnginePlugin(QObject *parent = nullptr);
    Q_DISABLE_COPY_MOVE(QHelpEnginePlugin)

    std::unique_ptr<QQmlLSHelpProviderBase> initialize(const QString &collectionFile,
                                                       QObject *parent) override;
};

QT_END_NAMESPACE

#endif // QHELPENGINEPLUGIN_H
