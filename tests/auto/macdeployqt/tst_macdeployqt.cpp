/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore>
#include <QtTest>

Q_LOGGING_CATEGORY(lcTests, "qt.tools.tests")

bool g_testDirectoryBuild = false; // toggle to keep build output for debugging.
QTemporaryDir *g_temporaryDirectory;
QString g_macdeployqtBinary;
QString g_qmakeBinary;
QString g_makeBinary;
QString g_installNameToolBinary;

#if QT_CONFIG(process)

static const QString msgProcessError(const QProcess &process, const QString &what)
{
    QString result;
    QTextStream(&result) << what << ": \"" << process.program() << ' '
        << process.arguments().join(QLatin1Char(' ')) << "\": " << process.errorString();
    return result;
}

static bool runProcess(const QString &binary,
                       const QStringList &arguments,
                       QString *errorMessage,
                       const QString &workingDir = QString(),
                       const QProcessEnvironment &env = QProcessEnvironment(),
                       int timeOut = 10000,
                       QByteArray *stdOut = nullptr, QByteArray *stdErr = nullptr)
{
    QProcess process;
    if (!env.isEmpty())
        process.setProcessEnvironment(env);
    if (!workingDir.isEmpty())
        process.setWorkingDirectory(workingDir);

    const auto outputReader = qScopeGuard([&] {
        QByteArray standardOutput = process.readAllStandardOutput();
        if (!standardOutput.trimmed().isEmpty())
            qCDebug(lcTests).nospace() << "Standard output:\n" << qUtf8Printable(standardOutput.trimmed());
        if (stdOut)
            *stdOut = standardOutput;
        QByteArray standardError = process.readAllStandardError();
        if (!standardError.trimmed().isEmpty())
            qCDebug(lcTests).nospace() << "Standard error:\n" << qUtf8Printable(standardError.trimmed());
        if (stdErr)
            *stdErr = standardError;
    });

    qCDebug(lcTests).noquote() << "Running" << binary
        << "with arguments" << arguments
        << "in" << workingDir;

    process.start(binary, arguments, QIODevice::ReadOnly);
    if (!process.waitForStarted()) {
        *errorMessage = msgProcessError(process, "Failed to start");
        return false;
    }
    if (!process.waitForFinished(timeOut)) {
        *errorMessage = msgProcessError(process, "Timed out");
        process.terminate();
        if (!process.waitForFinished(300))
            process.kill();
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit) {
        *errorMessage = msgProcessError(process, "Crashed");
        return false;
    }
    if (process.exitCode() != QProcess::NormalExit) {
        *errorMessage = msgProcessError(process, "Exit code " + QString::number(process.exitCode()));
        return false;
    }
    return true;
}

#else

static bool runProcess(const QString &binary,
                       const QStringList &arguments,
                       QString *arguments,
                       const QString &workingDir = QString(),
                       const QProcessEnvironment &env = QProcessEnvironment(),
                       int timeOut = 5000,
                       QByteArray *stdOut = Q_NULLPTR, QByteArray *stdErr = Q_NULLPTR)
{
    Q_UNUSED(binary);
    Q_UNUSED(arguments);
    Q_UNUSED(arguments);
    Q_UNUSED(workingDir);
    Q_UNUSED(env);
    Q_UNUSED(timeOut);
    Q_UNUSED(stdOut);
    Q_UNUSED(stdErr);
    return false;
}

#endif

QString sourcePath(const QString &name)
{
    return "source_" + name;
}

QString buildPath(const QString &name)
{
    if (g_testDirectoryBuild)
        return "build_" + name;
    return g_temporaryDirectory->path() + "/build_" + name;
}

bool qmake(const QString &source, const QString &destination, QString *errorMessage)
{
    QStringList args = QStringList() << source;
    return runProcess(g_qmakeBinary, args, errorMessage, destination);
}

bool make(const QString &destination, QString *errorMessage)
{
    QStringList args;
    return runProcess(g_makeBinary, args, errorMessage, destination,
                      {}, 60000);
}

void build(const QString &name)
{
    // Build the app or framework according to the convention used
    // by this test:
    //   source_name (source code)
    //   build_name  (build artifacts)

    QString source = sourcePath(name);
    QString build = buildPath(name);
    QString profile = name + ".pro";

    QString sourcePath = QFINDTESTDATA(source);
    QVERIFY(!sourcePath.isEmpty());

    // Clear/set up build dir
    QString buildPath = build;
    QVERIFY(QDir(buildPath).removeRecursively());
    QVERIFY(QDir().mkdir(buildPath));
    QVERIFY(QDir(buildPath).exists());

    // Build application
    QString sourceProFile = QDir(sourcePath).canonicalPath() + '/' + profile;
    QString errorMessage;
    QVERIFY2(qmake(sourceProFile, buildPath, &errorMessage), qPrintable(errorMessage));
    QVERIFY2(make(buildPath, &errorMessage), qPrintable(errorMessage));
}

bool changeInstallName(const QString &path, const QString &binary, const QString &from, const QString &to)
{
    QStringList args = QStringList() << binary << "-change" << from << to;
    QString errorMessage;
    return runProcess(g_installNameToolBinary, args, &errorMessage, path);
}

bool deploy(const QString &name, const QStringList &options, QString *errorMessage)
{
    QString bundle = name + ".app";
    QString path = buildPath(name);
    QStringList args = QStringList() << bundle << options;
    if (lcTests().isDebugEnabled())
        args << "-verbose=3";
    return runProcess(g_macdeployqtBinary, args, errorMessage, path);
}

bool run(const QString &name, QString *errorMessage)
{
    QString path = buildPath(name);
    QStringList args;
    QString binary = name + ".app/Contents/MacOS/" + name;
    return runProcess(binary, args, errorMessage, path);
}

bool runPrintLibraries(const QString &name, QString *errorMessage, QByteArray *stdErr)
{
    QString binary = name + ".app/Contents/MacOS/" + name;
    QString path = buildPath(name);
    QStringList args;
    QProcessEnvironment env = QProcessEnvironment();
    env.insert("DYLD_PRINT_LIBRARIES", "true");
    QByteArray stdOut;
    return runProcess(binary, args, errorMessage, path, env, 5000, &stdOut, stdErr);
}

void runVerifyDeployment(const QString &name)
{
    QString errorMessage;
    // Verify that the application runs after deployment and that it loads binaries from
    // the application bundle only.
    QByteArray libraries;
    QVERIFY2(runPrintLibraries(name, &errorMessage, &libraries), qPrintable(errorMessage));
    const QList<QString> parts = QString::fromLocal8Bit(libraries).split("dyld: loaded:");
    const QString qtPath = QLibraryInfo::path(QLibraryInfo::PrefixPath);
    // Let assume Qt is not installed in system
    foreach (QString part, parts) {
        part = part.trimmed();
        if (part.isEmpty())
            continue;
        QVERIFY(!parts.startsWith(qtPath));
    }
}

class tst_macdeployqt : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void basicapp();
};

void tst_macdeployqt::initTestCase()
{
#ifdef QT_NO_PROCESS
    QSKIP("This test requires QProcess support");
#endif

    // Set up test-global unique temporary directory
    g_temporaryDirectory = new QTemporaryDir();
    QVERIFY(g_temporaryDirectory->isValid());

    // Locate build and deployment tools
    g_macdeployqtBinary = QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/macdeployqt";
    QVERIFY(!g_macdeployqtBinary.isEmpty());
    g_qmakeBinary = QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/qmake";
    QVERIFY(!g_qmakeBinary.isEmpty());
    g_makeBinary = QStandardPaths::findExecutable("make");
    QVERIFY(!g_makeBinary.isEmpty());
    g_installNameToolBinary = QStandardPaths::findExecutable("install_name_tool");
    QVERIFY(!g_installNameToolBinary.isEmpty());
}

void tst_macdeployqt::cleanupTestCase()
{
    delete g_temporaryDirectory;
}

// Verify that deployment of a basic Qt Gui application works
void tst_macdeployqt::basicapp()
{
#ifdef QT_NO_PROCESS
    QSKIP("This test requires QProcess support");
#endif

    QString errorMessage;
    QString name = "basicapp";

    // Build and verify that the application runs before deployment
    build(name);
    QVERIFY2(run(name, &errorMessage), qPrintable(errorMessage));

    // Deploy application
    QVERIFY2(deploy(name, QStringList(), &errorMessage), qPrintable(errorMessage));

    // Verify deployment
    runVerifyDeployment(name);
}

QTEST_MAIN(tst_macdeployqt)
#include "tst_macdeployqt.moc"
