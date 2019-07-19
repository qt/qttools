/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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
#include <QProcess>
#include <QTemporaryDir>
#include <QtTest>

class tst_generatedOutput : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void htmlFromCpp();

private:
    QScopedPointer<QTemporaryDir> m_outputDir;
    QString m_qdoc;

    bool runQDocProcess(const QStringList &arguments);
    void compareLineByLine(QString &expectedFilename, QString &actualFilename);
};

void tst_generatedOutput::initTestCase()
{
    // Build the path to the QDoc binary the same way moc tests do for moc.
    const auto binpath = QLibraryInfo::location(QLibraryInfo::BinariesPath);
    const auto extension = QSysInfo::productType() == "windows" ? ".exe" : "";
    m_qdoc = binpath + QLatin1String("/qdoc") + extension;
}

void tst_generatedOutput::init()
{
    m_outputDir.reset(new QTemporaryDir());
    if (!m_outputDir->isValid()) {
        const QString errorMessage =
            "Couldn't create temporary directory: " + m_outputDir->errorString();
        QFAIL(qPrintable(errorMessage));
    }
}

bool tst_generatedOutput::runQDocProcess(const QStringList &arguments)
{
    QProcess qdocProcess;
    qdocProcess.setProgram(m_qdoc);
    qdocProcess.setArguments(arguments);
    qdocProcess.start();
    qdocProcess.waitForFinished();

    if (qdocProcess.exitCode() == 0)
        return true;

    QString output = qdocProcess.readAllStandardOutput();
    QString errors = qdocProcess.readAllStandardError();

    qInfo() << "QDoc exited with exit code" << qdocProcess.exitCode();
    if (output.size() > 0)
        qInfo() << "Received output:\n" << output;
    if (errors.size() > 0)
        qInfo() << "Received errors:\n" << errors;
    return false;
}

void tst_generatedOutput::compareLineByLine(QString &expected, QString &actual)
{
    QFile expectedFile(expected);
    if (!expectedFile.open(QIODevice::ReadOnly))
        QFAIL("Cannot open expected data file!");
    QTextStream expectedIn(&expectedFile);

    QFile actualFile(actual);
    if (!actualFile.open(QIODevice::ReadOnly))
        QFAIL("Cannot open actual data file!");
    QTextStream actualIn(&actualFile);

    int lineNumber = 0;
    while (!expectedIn.atEnd() && !actualIn.atEnd()) {
        lineNumber++;
        QString prefix = QString::number(lineNumber) + QLatin1String(": ");
        QString expectedLine = prefix + expectedIn.readLine();
        QString actualLine = prefix + actualIn.readLine();
        QCOMPARE(actualLine, expectedLine);
    }
}

void tst_generatedOutput::htmlFromCpp()
{
    const QStringList arguments = {
            "--outputdir",
            m_outputDir->path(),
            QFINDTESTDATA("test.qdocconf")
    };

    if (!runQDocProcess(arguments))
        QFAIL("Running QDoc failed. See output above.");

    QString expectedFile = QFINDTESTDATA("/expected_output/qdoctests-qdocfileoutput.html");
    QString actualFile = m_outputDir->path() + QLatin1String("/qdoctests-qdocfileoutput.html");

    compareLineByLine(expectedFile, actualFile);
}
QTEST_APPLESS_MAIN(tst_generatedOutput)

#include "tst_generatedoutput.moc"
