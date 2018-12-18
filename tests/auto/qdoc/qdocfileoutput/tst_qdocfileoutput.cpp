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

class tstQDocFileOutput : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void compareQDocOutputWithExpectedData();

private:
    QTemporaryDir m_outputDir;
};

void tstQDocFileOutput::initTestCase()
{
    if (!m_outputDir.isValid()) {
        const QString errorMessage =
            "Couldn't create temporary directory:" + m_outputDir.errorString();
        QFAIL(qPrintable(errorMessage));
    }
}

void tstQDocFileOutput::compareQDocOutputWithExpectedData()
{
    // Build the path to the QDoc binary the same way moc tests do for moc.
    QString binpath = QLibraryInfo::location(QLibraryInfo::BinariesPath);
    QString extension;
    if (QSysInfo::productType() == "windows")
        extension += ".exe";
    const QString qdoc = QString("%1/qdoc" + extension).arg(binpath);

    const QStringList arguments = {
            "--outputdir",
            m_outputDir.path(),
            QFINDTESTDATA("test.qdocconf")
        };

    QProcess qdocProcess;
    qdocProcess.setProgram(qdoc);
    qdocProcess.setArguments(arguments);
    qdocProcess.start();
    qdocProcess.waitForFinished();
    QString output(qdocProcess.readAllStandardOutput());
    QString errors(qdocProcess.readAllStandardError());

    if (qdocProcess.exitCode() != 0) {
        qInfo() << "QDoc exited with exit code" << qdocProcess.exitCode();
        if (output.size() > 0)
            qInfo() << "Received output:\n" << output;
        if (errors.size() > 0)
            qInfo() << "Received errors:\n" << errors;
        QFAIL("Running QDoc failed. See output above.");
    }

    QFile expected(QFINDTESTDATA("/expected_output/qdoctests-qdocfileoutput.html"));
    QFile actual(m_outputDir.path() + "/qdoctests-qdocfileoutput.html");

    if (!expected.open(QIODevice::ReadOnly))
        QFAIL("Cannot open expected data file!");
    QTextStream expectedIn(&expected);

    if (!actual.open(QIODevice::ReadOnly))
        QFAIL("Cannot open actual data file!");
    QTextStream actualIn(&actual);

    int lineNumber = 0;
    while (!expectedIn.atEnd()) {
        while (!actualIn.atEnd()) {
            lineNumber++;
            QString expectedLine =
                QString::number(lineNumber) + ": " + expectedIn.readLine();
            QString actualLine =
                QString::number(lineNumber) + ": " + actualIn.readLine();
            QCOMPARE(expectedLine, actualLine);
        }
    }
}

QTEST_APPLESS_MAIN(tstQDocFileOutput)

#include "tst_qdocfileoutput.moc"
