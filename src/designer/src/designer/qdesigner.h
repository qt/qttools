// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

#ifndef QDESIGNER_H
#define QDESIGNER_H

#include "helpclient.h"

#include <QtWidgets/qapplication.h>

#include <QtCore/qlibraryinfo.h>
#include <QtCore/qpointer.h>
#include <QtCore/qversionnumber.h>

#include <optional>

QT_BEGIN_NAMESPACE

#define qDesigner \
    (static_cast<QDesigner*>(QCoreApplication::instance()))

class QDesignerWorkbench;
class QDesignerToolWindow;
class MainWindowBase;
class QErrorMessage;
class QCommandLineParser;
struct Options;

struct Options
{
    QStringList files;
    QString resourceDir{QLibraryInfo::paths(QLibraryInfo::TranslationsPath).value(0)};
    QStringList pluginPaths;
    std::optional<QVersionNumber> qtVersion;
    QString localSocketServerName;
    bool server{false};
    quint16 tcpClientPort{0};
    bool enableInternalDynamicProperties{false};
    HelpClientType helpMode{HelpClientType::Assistant};
};

class QDesigner: public QApplication
{
    Q_OBJECT
public:
    enum ParseArgumentsResult {
        ParseArgumentsSuccess,
        ParseArgumentsError,
        ParseArgumentsHelpRequested
    };

    QDesigner(int &argc, char **argv);
    ~QDesigner() override;

    ParseArgumentsResult parseCommandLineArguments();

    QDesignerWorkbench *workbench() const;
    bool isServerOrClientEnabled() const;
    MainWindowBase *mainWindow() const;
    void setMainWindow(MainWindowBase *tw);

protected:
    bool event(QEvent *ev) override;

signals:
    void initialized();

public slots:
    void showErrorMessage(const QString &message);

private:
    void showErrorMessageBox(const QString &);

    QObject *m_server = nullptr;
    QObject *m_client = nullptr;
    QDesignerWorkbench *m_workbench = nullptr;
    QPointer<MainWindowBase> m_mainWindow;
    QPointer<QErrorMessage> m_errorMessageDialog;

    QString m_initializationErrors;
    QString m_lastErrorMessage;
};

QT_END_NAMESPACE

#endif // QDESIGNER_H
