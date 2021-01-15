/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef RUNNER_H
#define RUNNER_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QScopedPointer>
#include <QtCore/QLoggingCategory>

QT_USE_NAMESPACE

class RunnerPrivate;
class Runner
{
public:
    static QMap<QString, QStringList> deviceNames();

    Runner(const QString &app, const QStringList &arguments, const QString &profile = QString(),
           const QString &device = QString());
    ~Runner();

    bool isValid() const;
    QString app() const;
    QStringList arguments() const;
    int deviceIndex() const;

    bool install(bool removeFirst = false);
    bool remove();
    bool start();
    bool enableDebugging(const QString &debuggerExecutable, const QString &debuggerArguments);
    bool disableDebugging();
    bool setLoopbackExemptClientEnabled(bool enabled);
    bool setLoopbackExemptServerEnabled(bool enabled);
    bool setLoggingRules(const QByteArray &rules);
    bool suspend();
    bool stop();
    bool wait(int maxWaitTime = 0);
    bool setupTest();
    bool collectTest();
    qint64 pid();
    int exitCode();

private:
    QScopedPointer<RunnerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Runner)
};

Q_DECLARE_LOGGING_CATEGORY(lcWinRtRunner)
Q_DECLARE_LOGGING_CATEGORY(lcWinRtRunnerApp)

#endif // RUNNER_H
