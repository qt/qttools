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

#ifndef APPXLOCALENGINE_H
#define APPXLOCALENGINE_H

#include "appxengine.h"
#include "runner.h"

#include <QtCore/QScopedPointer>
#include <QtCore/QString>

QT_USE_NAMESPACE

class AppxLocalEnginePrivate;
class AppxLocalEngine : public AppxEngine
{
public:
    static bool canHandle(Runner *runner);
    static RunnerEngine *create(Runner *runner);
    static QStringList deviceNames();

    bool install(bool removeFirst = false) override;
    bool remove() override;
    bool start() override;
    bool enableDebugging(const QString &debuggerExecutable,
                        const QString &debuggerArguments) override;
    bool disableDebugging() override;
    bool setLoopbackExemptClientEnabled(bool enabled) override;
    bool setLoopbackExemptServerEnabled(bool enabled) override;
    bool setLoggingRules(const QByteArray &rules) override;
    bool suspend() override;
    bool waitForFinished(int secs) override;
    bool stop() override;

    QString devicePath(const QString &relativePath) const override;
    bool sendFile(const QString &localFile, const QString &deviceFile) override;
    bool receiveFile(const QString &deviceFile, const QString &localFile) override;

private:
    explicit AppxLocalEngine(Runner *runner);
    ~AppxLocalEngine();

    QString extensionSdkPath() const override;
    bool installPackage(IAppxManifestReader *reader, const QString &filePath) override;

    bool parseExitCode();
    friend struct QScopedPointerDeleter<AppxLocalEngine>;
    Q_DECLARE_PRIVATE(AppxLocalEngine)
};

#endif // APPXLOCALENGINE_H
