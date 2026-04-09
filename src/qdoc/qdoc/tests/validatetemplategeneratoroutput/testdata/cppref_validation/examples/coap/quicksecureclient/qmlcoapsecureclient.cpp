// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "qmlcoapsecureclient.h"

#include <QCoapClient>
#include <QCoapReply>
#include <QFile>
#include <QLoggingCategory>
#include <QMetaEnum>
#include <QUrl>

Q_STATIC_LOGGING_CATEGORY(lcCoapClient, "qt.coap.client")

QmlCoapSecureClient::QmlCoapSecureClient(QObject *parent)
    : QObject(parent)
    , m_coapClient(nullptr)
    , m_securityMode(QtCoap::SecurityMode::NoSecurity)
{
}

QmlCoapSecureClient::~QmlCoapSecureClient()
{
    if (m_coapClient) {
        delete m_coapClient;
        m_coapClient = nullptr;
    }
}

static QString errorMessage(QtCoap::Error errorCode)
{
    const auto error = QMetaEnum::fromType<QtCoap::Error>().valueToKey(static_cast<int>(errorCode));
    return QmlCoapSecureClient::tr("Request failed with error: %1\n").arg(error);
}

//! [set_security_mode]
void QmlCoapSecureClient::setSecurityMode(QtCoap::SecurityMode mode)
{
    // Create a new client, if the security mode has changed
    if (m_coapClient && mode != m_securityMode) {
        delete m_coapClient;
        m_coapClient = nullptr;
    }

    if (!m_coapClient) {
        m_coapClient = new QCoapClient(mode);
        m_securityMode = mode;

        connect(m_coapClient, &QCoapClient::finished, this,
                [this](QCoapReply *reply) {
                    if (!reply)
                        emit finished(tr("Something went wrong, received a null reply"));
                    else if (reply->errorReceived() != QtCoap::Error::Ok)
                        emit finished(errorMessage(reply->errorReceived()));
                    else
                        emit finished(reply->message().payload());
                });

        connect(m_coapClient, &QCoapClient::error, this,
                [this](QCoapReply *, QtCoap::Error errorCode) {
                    emit finished(errorMessage(errorCode));
                });
    }
}
//! [set_security_mode]

//! [send_get_request]
void QmlCoapSecureClient::sendGetRequest(const QString &host, const QString &path, int port)
{
    if (!m_coapClient)
        return;

    m_coapClient->setSecurityConfiguration(m_configuration);

    QUrl url;
    url.setHost(host);
    url.setPath(path);
    url.setPort(port);
    m_coapClient->get(url);
}
//! [send_get_request]

//! [set_configuration_psk]
void
QmlCoapSecureClient::setSecurityConfiguration(const QString &preSharedKey, const QString &identity)
{
    QCoapSecurityConfiguration configuration;
    configuration.setPreSharedKey(preSharedKey.toUtf8());
    configuration.setPreSharedKeyIdentity(identity.toUtf8());
    m_configuration = configuration;
}
//! [set_configuration_psk]

//! [set_configuration_cert]
void QmlCoapSecureClient::setSecurityConfiguration(const QString &localCertificatePath,
                                                   const QString &caCertificatePath,
                                                   const QString &privateKeyPath)
{
    QCoapSecurityConfiguration configuration;

    const auto localCerts =
            QSslCertificate::fromPath(QUrl(localCertificatePath).toLocalFile(), QSsl::Pem,
                                      QSslCertificate::PatternSyntax::FixedString);
    if (localCerts.isEmpty())
        qCWarning(lcCoapClient, "The specified local certificate file is not valid.");
    else
        configuration.setLocalCertificateChain(localCerts.toVector());

    const auto caCerts = QSslCertificate::fromPath(QUrl(caCertificatePath).toLocalFile(), QSsl::Pem,
                                                   QSslCertificate::PatternSyntax::FixedString);
    if (caCerts.isEmpty())
        qCWarning(lcCoapClient, "The specified CA certificate file is not valid.");
    else
        configuration.setCaCertificates(caCerts.toVector());

    QFile privateKey(QUrl(privateKeyPath).toLocalFile());
    if (privateKey.open(QIODevice::ReadOnly)) {
        QCoapPrivateKey key(privateKey.readAll(), QSsl::Ec);
        configuration.setPrivateKey(key);
    } else {
        qCWarning(lcCoapClient) << "Unable to read the specified private key file"
                                << privateKeyPath;
    }
    m_configuration = configuration;
}
//! [set_configuration_cert]

//! [disconnect]
void QmlCoapSecureClient::disconnect()
{
    if (m_coapClient)
        m_coapClient->disconnect();
}
//! [disconnect]
