/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QDESIGNER_H
#define QDESIGNER_H

#include <QtCore/qpointer.h>
#include <QtWidgets/qapplication.h>

QT_BEGIN_NAMESPACE

#define qDesigner \
    (static_cast<QDesigner*>(QCoreApplication::instance()))

class QDesignerWorkbench;
class QDesignerToolWindow;
class MainWindowBase;
class QDesignerServer;
class QDesignerClient;
class QErrorMessage;
class QCommandLineParser;
struct Options;

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
    QDesignerServer *server() const;
    MainWindowBase *mainWindow() const;
    void setMainWindow(MainWindowBase *tw);

protected:
    bool event(QEvent *ev) override;

signals:
    void initialized();

public slots:
    void showErrorMessage(const QString &message);

private slots:
    void callCreateForm();

private:
    void showErrorMessageBox(const QString &);

    QDesignerServer *m_server;
    QDesignerClient *m_client;
    QDesignerWorkbench *m_workbench;
    QPointer<MainWindowBase> m_mainWindow;
    QPointer<QErrorMessage> m_errorMessageDialog;

    QString m_initializationErrors;
    QString m_lastErrorMessage;
    bool m_suppressNewFormShow;
};

QT_END_NAMESPACE

#endif // QDESIGNER_H
