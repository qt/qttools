// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QTDOCINSTALLER
#define QTDOCINSTALLER

#include <QtCore/QDir>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QStringList>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

class HelpEngineWrapper;

class QtDocInstaller : public QThread
{
    Q_OBJECT

public:
    typedef QPair<QString, QStringList> DocInfo;
    QtDocInstaller(const QList<DocInfo> &docInfos);
    ~QtDocInstaller() override;
    void installDocs();

signals:
    void qchFileNotFound(const QString &component);
    void registerDocumentation(const QString &component,
                               const QString &absFileName);
    void docsInstalled(bool newDocsInstalled);

private:
    void run() override;
    bool installDoc(const DocInfo &docInfo);

    bool m_abort;
    QMutex m_mutex;
    QStringList m_qchFiles;
    QDir m_qchDir;
    QList<DocInfo> m_docInfos;
};

QT_END_NAMESPACE

#endif
