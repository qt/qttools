// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CMDLINEPARSER_H
#define CMDLINEPARSER_H

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class CmdLineParser
{
    Q_DECLARE_TR_FUNCTIONS(CmdLineParser)
public:
    enum Result {Ok, Help, Error};
    enum ShowState {Untouched, Show, Hide, Activate};
    enum RegisterState {None, Register, Unregister};

    CmdLineParser(const QStringList &arguments);
    Result parse();

    void setCollectionFile(const QString &file);
    QString collectionFile() const;
    bool collectionFileGiven() const;
    QString cloneFile() const;
    QUrl url() const;
    bool enableRemoteControl() const;
    ShowState contents() const;
    ShowState index() const;
    ShowState bookmarks() const;
    ShowState search() const;
    QString currentFilter() const;
    bool removeSearchIndex() const;
    bool rebuildSearchIndex() const;
    RegisterState registerRequest() const;
    QString helpFile() const;

    void showMessage(const QString &msg, bool error);

private:
    QString getFileName(const QString &fileName);
    bool hasMoreArgs() const;
    const QString &nextArg();
    void handleCollectionFileOption();
    void handleShowUrlOption();
    void handleShowOption();
    void handleHideOption();
    void handleActivateOption();
    void handleShowOrHideOrActivateOption(ShowState state);
    void handleRegisterOption();
    void handleUnregisterOption();
    void handleRegisterOrUnregisterOption(RegisterState state);
    void handleSetCurrentFilterOption();

    QStringList m_arguments;
    int m_pos;
    QString m_collectionFile;
    QString m_cloneFile;
    QString m_helpFile;
    QUrl m_url;
    bool m_enableRemoteControl;

    ShowState m_contents;
    ShowState m_index;
    ShowState m_bookmarks;
    ShowState m_search;
    RegisterState m_register;
    QString m_currentFilter;
    bool m_removeSearchIndex;
    bool m_quiet;
    QString m_error;
};

QT_END_NAMESPACE

#endif
