// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HELPCLIENT_H
#define HELPCLIENT_H

#include <QtCore/qtclasshelpermacros.h>

QT_BEGIN_NAMESPACE

class QString;

class HelpClient
{
public:
    Q_DISABLE_COPY_MOVE(HelpClient)

    HelpClient() noexcept = default;
    virtual ~HelpClient() = default;

    virtual bool showPage(const QString &path, QString *errorMessage) = 0;
    virtual bool activateIdentifier(const QString &identifier, QString *errorMessage) = 0;
    virtual QString documentUrl(const QString &module) const = 0;

    // Root of the Qt Widgets Designer documentation
    QString designerManualUrl() const;
};

class WebHelpClient : public HelpClient
{
public:
    Q_DISABLE_COPY_MOVE(WebHelpClient)
    WebHelpClient() noexcept = default;
    ~WebHelpClient() override = default;

    bool showPage(const QString &path, QString *errorMessage) override;
    bool activateIdentifier(const QString &identifier, QString *errorMessage) override;
    QString documentUrl(const QString &module) const override;

private:
    QString webPage(const QString &identifier);
};

class PythonWebHelpClient : public WebHelpClient
{
public:
    Q_DISABLE_COPY_MOVE(PythonWebHelpClient)
    PythonWebHelpClient() noexcept = default;
    ~PythonWebHelpClient() override = default;

    bool activateIdentifier(const QString &identifier, QString *errorMessage) override;

private:
    QString webPage(const QString &identifier);
};

QT_END_NAMESPACE

#endif // HELPCLIENT_H
