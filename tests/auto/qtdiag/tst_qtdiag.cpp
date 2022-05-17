// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include <QtTest/QtTest>

#include <QtCore/QProcess>
#include <QtCore/QLibraryInfo>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

class tst_QtDiag : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void run();

private:
    QString m_binary;
};

void tst_QtDiag::initTestCase()
{
    QString binary = QLibraryInfo::path(QLibraryInfo::BinariesPath) + QStringLiteral("/qtdiag");
#  ifdef Q_OS_WIN
    binary += QStringLiteral(".exe");
#  endif
    const QFileInfo fi(binary);
    if (fi.isFile()) {
        m_binary = fi.absoluteFilePath();
    } else {
        const QByteArray message = QByteArrayLiteral("The binary '")
            + QDir::toNativeSeparators(binary).toLocal8Bit()
            +  QByteArrayLiteral("' does not exist.");
        QSKIP(message.constData());
    }
}

void tst_QtDiag::run()
{
    if (m_binary.isEmpty())
        QSKIP("Binary could not be found");
    QProcess process;
    qDebug() << "Launching " << QDir::toNativeSeparators(m_binary);
    process.start(m_binary, QStringList{});
    QVERIFY2(process.waitForStarted(), qPrintable(process.errorString()));
    QVERIFY(process.waitForFinished());
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), 0);
    QByteArray output = process.readAll();
    QVERIFY(!output.isEmpty());
    output.replace('\r', "");
    qDebug("\n%s", output.constData());
}

QTEST_MAIN(tst_QtDiag)
#include "tst_qtdiag.moc"
