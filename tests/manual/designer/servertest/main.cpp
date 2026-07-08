// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QTimer>

#include <memory>

using namespace Qt::StringLiterals;

template <class Server>
void sendFiles(Server &server, const QStringList &files)
{
    if (auto *socket = server.nextPendingConnection()) {
        for (const auto &f : files)
            socket->write(f.toUtf8() + "\r\n");
        socket->flush();
        socket->close();
        QTimer::singleShot(0, QCoreApplication::quit);
    }
}

class LocalServer : public QObject
{
public:
    explicit LocalServer(const QString &serverName, QStringList files,
                         QObject *parent = nullptr) : QObject(parent), m_files(std::move(files))
    {
        connect(&m_server, &QLocalServer::newConnection, this, &LocalServer::handleNewConnection);
        m_server.listen(serverName);
        qInfo() << "(Local Socket) Listening on" << serverName;
    }

private Q_SLOTS:
    void handleNewConnection()
    {
        qInfo("%s", Q_FUNC_INFO);
        sendFiles(m_server, m_files);
    }

private:
    QLocalServer m_server;
    QStringList m_files;
};

class TcpServer : public QObject
{
public:
    explicit TcpServer(quint16 port, QStringList files,
                       QObject *parent = nullptr) : QObject(parent),  m_files(std::move(files))
    {
        connect(&m_server, &QTcpServer::newConnection, this, &TcpServer::handleNewConnection);
        m_server.listen(QHostAddress::LocalHost, port);
        qInfo() << "(TCP) Listening on " << m_server.serverPort();
    }

private Q_SLOTS:
    void handleNewConnection()
    {
        qInfo("%s", Q_FUNC_INFO);
        sendFiles(m_server, m_files);
    }

private:
    QTcpServer m_server;
    QStringList m_files;
};

static constexpr auto description = R"(Qt Widgets Designer test server

Pass some .ui files and then launch Qt Widgets Designer test server
with the respective client option.
)"_L1;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    QCoreApplication::setApplicationVersion(QLatin1StringView(qVersion()));
    parser.setApplicationDescription(description);
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption serverNameOption(u"name"_s, u"Local socket server_name"_s,
                                              u"server_name"_s, u"test"_s);
    parser.addOption(serverNameOption);
    const QCommandLineOption tcpPortOption(u"port"_s, u"TCP Server port"_s, u"port"_s);
    parser.addOption(tcpPortOption);
    parser.addPositionalArgument(u"file(s)"_s, u".ui Files to load"_s);
    parser.process(app);

    QStringList files = parser.positionalArguments();
    if (files.isEmpty())
        parser.showHelp(0);

    std::unique_ptr<QObject> server;
    if (parser.isSet(tcpPortOption))
        server= std::make_unique<TcpServer>(parser.value(tcpPortOption).toInt(), files);
    else
        server= std::make_unique<LocalServer>(parser.value(serverNameOption), files);

    return QCoreApplication::exec();
}
