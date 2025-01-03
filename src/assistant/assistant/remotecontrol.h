// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class HelpEngineWrapper;
class MainWindow;

class RemoteControl : public QObject
{
    Q_OBJECT

public:
    RemoteControl(MainWindow *mainWindow);

private slots:
    void handleCommandString(const QString &cmdString);
    void applyCache();

private:
    void clearCache();
    void splitInputString(const QString &input, QString &cmd, QString &arg);
    void handleDebugCommand(const QString &arg);
    void handleShowOrHideCommand(const QString &arg, bool show);
    void handleSetSourceCommand(const QString &arg);
    void handleSyncContentsCommand();
    void handleActivateKeywordCommand(const QString &arg);
    void handleActivateIdentifierCommand(const QString &arg);
    void handleExpandTocCommand(const QString &arg);
    void handleSetCurrentFilterCommand(const QString &arg);
    void handleRegisterCommand(const QString &arg);
    void handleUnregisterCommand(const QString &arg);

private:
    MainWindow *m_mainWindow;
    QUrl m_setSource;
    QString m_activateKeyword;
    QString m_activateIdentifier;
    QString m_currentFilter;
    HelpEngineWrapper &helpEngine;
    int m_expandTOC = -2;
    bool m_debug = false;

    bool m_caching = true;
    bool m_syncContents = false;
};

QT_END_NAMESPACE

#endif
