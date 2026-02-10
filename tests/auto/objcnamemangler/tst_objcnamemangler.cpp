// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QFile>
#include <QLibraryInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>

#include <QtCore/private/qexpected_p.h>

using namespace Qt::Literals::StringLiterals;
using namespace std::chrono_literals;

class tst_objcnamemangler : public QObject
{
    Q_OBJECT

public:
    tst_objcnamemangler();

private slots:
    void initTestCase();

    void replaceSuffix();
    void replacePrefix();
    void replaceInfix();
    void excludeClass();
    void dryRun();

private:
    struct ProcessResult {
        int exitCode;
        QString stdOut;
        QString stdErr;
    };

    q23::expected<ProcessResult, QString> runProcess(const QString &program,
                                                     const QStringList &arguments,
                                                     std::chrono::milliseconds timeout = 5s);

    q23::expected<void, QString> runObjcNameMangler(const QStringList &args);
    q23::expected<QString, QString> runTestExecutable(const QString &binaryPath);
    q23::expected<void, QString> codesignBinary(const QString &binaryPath);
    q23::expected<QString, QString> copyTestBinary(const QString &suffix);

    QString objcnamemangler;
    QString testBinary;
    QTemporaryDir tempDir;
};

tst_objcnamemangler::tst_objcnamemangler()
    : objcnamemangler(QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath)
                      + u"/objcnamemangler"_s)
{
}

void tst_objcnamemangler::initTestCase()
{
    // Verify objcnamemangler tool exists
    QVERIFY2(QFile::exists(objcnamemangler),
             qPrintable(u"objcnamemangler not found at: %1"_s.arg(objcnamemangler)));

    // Find the test binary built by CMake
    testBinary = QCoreApplication::applicationDirPath() + u"/objc_test_binary"_s;
    QVERIFY2(QFile::exists(testBinary),
             qPrintable(u"Test binary not found at: %1"_s.arg(testBinary)));

    // Verify temporary directory is valid
    QVERIFY2(tempDir.isValid(),
             qPrintable(u"Failed to create temporary directory: %1"_s.arg(tempDir.errorString())));
}

q23::expected<tst_objcnamemangler::ProcessResult, QString>
tst_objcnamemangler::runProcess(const QString &program, const QStringList &arguments,
                                std::chrono::milliseconds timeout)
{
    QProcess proc;
    proc.start(program, arguments);
    int timeoutMs = int(timeout.count());

    if (!proc.waitForStarted())
        return q23::unexpected(u"Failed to start %1: %2"_s
                              .arg(program, proc.errorString()));

    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        proc.waitForFinished();
        return q23::unexpected(u"%1 timed out after %2ms"_s
                              .arg(program).arg(timeoutMs));
    }

    if (proc.exitStatus() != QProcess::NormalExit)
        return q23::unexpected(u"%1 crashed"_s.arg(program));

    ProcessResult result;
    result.exitCode = proc.exitCode();
    result.stdOut = QString::fromUtf8(proc.readAllStandardOutput());
    result.stdErr = QString::fromUtf8(proc.readAllStandardError());

    return result;
}

q23::expected<void, QString> tst_objcnamemangler::runObjcNameMangler(const QStringList &args)
{
    auto result = runProcess(objcnamemangler, args);
    if (!result.has_value())
        return q23::unexpected(result.error());

    if (result->exitCode != 0) {
        return q23::unexpected(u"objcnamemangler failed (exit code %1): %2"_s
                              .arg(result->exitCode).arg(result->stdErr));
    }

    return {};
}

q23::expected<QString, QString> tst_objcnamemangler::runTestExecutable(const QString &binaryPath)
{
    auto result = runProcess(binaryPath, QStringList(), 3s);
    if (!result.has_value())
        return q23::unexpected(result.error());

    if (result->exitCode != 0) {
        return q23::unexpected(u"Test executable failed with exit code %1"_s
                              .arg(result->exitCode));
    }

    return result->stdOut.trimmed();
}

q23::expected<void, QString> tst_objcnamemangler::codesignBinary(const QString &binaryPath)
{
    QStringList args{
        u"--sign"_s,
        u"-"_s,
        binaryPath,
    };

    auto result = runProcess(u"codesign"_s, args);
    if (!result.has_value())
        return q23::unexpected(result.error());

    if (result->exitCode != 0) {
        return q23::unexpected(u"codesign failed (exit code %1): %2"_s
                              .arg(result->exitCode).arg(result->stdErr));
    }

    return {};
}

q23::expected<QString, QString> tst_objcnamemangler::copyTestBinary(const QString &suffix)
{
    QString destPath = tempDir.filePath(u"test_binary_%1"_s.arg(suffix));

    if (!QFile::copy(testBinary, destPath))
        return q23::unexpected(u"Failed to copy test binary to %1"_s.arg(destPath));

    // Set executable permissions
    QFile destFile(destPath);
    if (!destFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                  QFile::ReadGroup | QFile::ExeGroup |
                                  QFile::ReadOther | QFile::ExeOther)) {
        return q23::unexpected(u"Failed to set executable permissions on %1"_s.arg(destPath));
    }

    return destPath;
}

void tst_objcnamemangler::replaceSuffix()
{
    // Copy test binary
    auto testCopyResult = copyTestBinary(u"suffix"_s);
    if (!testCopyResult.has_value())
        QFAIL(qPrintable(testCopyResult.error()));
    QString testCopy = testCopyResult.value();

    // Run objcnamemangler to replace suffix
    QStringList args{
        u"--replace"_s,
        u"_Suffix"_s,
        u"_SUFFIX"_s,
        testCopy,
    };
    auto manglerResult = runObjcNameMangler(args);
    if (!manglerResult.has_value())
        QFAIL(qPrintable(manglerResult.error()));

    // Re-sign the binary
    auto codesignResult = codesignBinary(testCopy);
    if (!codesignResult.has_value())
        QFAIL(qPrintable(codesignResult.error()));

    // Run modified binary and verify output
    auto outputResult = runTestExecutable(testCopy);
    if (!outputResult.has_value())
        QFAIL(qPrintable(outputResult.error()));
    QCOMPARE(outputResult.value(), u"TestClass_SUFFIX"_s);
}

void tst_objcnamemangler::replacePrefix()
{
    // Copy test binary
    auto testCopyResult = copyTestBinary(u"prefix"_s);
    if (!testCopyResult.has_value())
        QFAIL(qPrintable(testCopyResult.error()));
    QString testCopy = testCopyResult.value();

    // Run objcnamemangler to replace prefix
    QStringList args{
        u"--replace"_s,
        u"TestClass_"_s,
        u"TestKlass_"_s,
        testCopy,
    };
    auto manglerResult = runObjcNameMangler(args);
    if (!manglerResult.has_value())
        QFAIL(qPrintable(manglerResult.error()));

    // Re-sign the binary
    auto codesignResult = codesignBinary(testCopy);
    if (!codesignResult.has_value())
        QFAIL(qPrintable(codesignResult.error()));

    // Run modified binary and verify output
    auto outputResult = runTestExecutable(testCopy);
    if (!outputResult.has_value())
        QFAIL(qPrintable(outputResult.error()));
    QCOMPARE(outputResult.value(), u"TestKlass_Suffix"_s);
}

void tst_objcnamemangler::replaceInfix()
{
    // Copy test binary
    auto testCopyResult = copyTestBinary(u"infix"_s);
    if (!testCopyResult.has_value())
        QFAIL(qPrintable(testCopyResult.error()));
    QString testCopy = testCopyResult.value();

    // Run objcnamemangler to replace infix
    QStringList args{
        u"--replace"_s,
        u"Class_"_s,
        u"Qlass_"_s,
        testCopy,
    };
    auto manglerResult = runObjcNameMangler(args);
    if (!manglerResult.has_value())
        QFAIL(qPrintable(manglerResult.error()));

    // Re-sign the binary
    auto codesignResult = codesignBinary(testCopy);
    if (!codesignResult.has_value())
        QFAIL(qPrintable(codesignResult.error()));

    // Run modified binary and verify output
    auto outputResult = runTestExecutable(testCopy);
    if (!outputResult.has_value())
        QFAIL(qPrintable(outputResult.error()));
    QCOMPARE(outputResult.value(), u"TestQlass_Suffix"_s);
}

void tst_objcnamemangler::excludeClass()
{
    // Copy test binary
    auto testCopyResult = copyTestBinary(u"exclude"_s);
    if (!testCopyResult.has_value())
        QFAIL(qPrintable(testCopyResult.error()));
    QString testCopy = testCopyResult.value();

    // Run objcnamemangler with --exclude flag
    // This should NOT modify the class because it's excluded
    QStringList args{
        u"--exclude"_s, u"TestClass_Suffix"_s,
        u"--replace"_s, u"_Suffix"_s, u"_SUFFIX"_s,
        testCopy
    };
    auto manglerResult = runObjcNameMangler(args);
    if (!manglerResult.has_value())
        QFAIL(qPrintable(manglerResult.error()));

    // Re-sign the binary
    auto codesignResult = codesignBinary(testCopy);
    if (!codesignResult.has_value())
        QFAIL(qPrintable(codesignResult.error()));

    // Run modified binary and verify output - should be unchanged
    auto outputResult = runTestExecutable(testCopy);
    if (!outputResult.has_value())
        QFAIL(qPrintable(outputResult.error()));
    QCOMPARE(outputResult.value(), u"TestClass_Suffix"_s);
}

void tst_objcnamemangler::dryRun()
{
    // Copy test binary
    auto testCopyResult = copyTestBinary(u"dryrun"_s);
    if (!testCopyResult.has_value())
        QFAIL(qPrintable(testCopyResult.error()));
    QString testCopy = testCopyResult.value();

    // Run objcnamemangler with --dry-run flag
    // This should NOT modify the binary
    QStringList args{
        u"--dry-run"_s,
        u"--exclude"_s, u"TestClass_Suffix"_s,
        u"--replace"_s, u"_Suffix"_s, u"_SUFFIX"_s,
        testCopy
    };
    auto manglerResult = runObjcNameMangler(args);
    if (!manglerResult.has_value())
        QFAIL(qPrintable(manglerResult.error()));

    // No need to re-sign since binary wasn't modified

    // Run binary and verify output - should be unchanged
    auto outputResult = runTestExecutable(testCopy);
    if (!outputResult.has_value())
        QFAIL(qPrintable(outputResult.error()));
    QCOMPARE(outputResult.value(), u"TestClass_Suffix"_s);
}

QTEST_MAIN(tst_objcnamemangler)

#include "tst_objcnamemangler.moc"
