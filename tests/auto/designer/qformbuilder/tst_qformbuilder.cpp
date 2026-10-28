// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtDesigner/QFormBuilder>

#include <QtWidgets/qgraphicsview.h>

#include <QtTest/qtest.h>

#include <QtCore/qbuffer.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qstring.h>

#include <algorithm>
#include <memory>
#include <unordered_set>

using namespace Qt::StringLiterals;

static const char writeBaselineEnvVar[] = "WRITE_BASELINE";
static const char qpaPlatform[] = "offscreen";

using CreationFunction = std::unique_ptr<QWidget>(*)();
Q_DECLARE_METATYPE(CreationFunction);

static QString msgCannotOpen(const QFile &file)
{
    return "Cannot open "_L1 + QDir::toNativeSeparators(file.fileName()) + ": "_L1
            + file.errorString();
}

// Poor man's diff reporting the first difference
static QByteArray msgDiff(const QByteArray &actualText, const QByteArray &expectedText)
{
    QByteArray result;

    const QByteArrayList actual = actualText.split('\n');
    const QByteArrayList expected = expectedText.split('\n');
    const auto [aIt, eIt] = std::mismatch(actual.cbegin(), actual.cend(),
                                          expected.cbegin(), expected.cend());
    if (aIt == actual.cend() && eIt == expected.cend())
        return result;

    if (actual.size() != expected.size()) {
        result += "Lines: " + QByteArray::number(actual.size()) + " != "
                  + QByteArray::number(expected.size()) + ", ";
    }

    const auto line = aIt - actual.cbegin();
    result += "first difference at line " + QByteArray::number(line + 1) + ":\n";

    for (auto contextLine = std::max(qsizetype(0), line - 2); contextLine < line; ++contextLine)
        result += ' ' + actual.at(contextLine) + '\n';
    if (eIt != expected.cend())
        result += '-' + *eIt + '\n';
    if (aIt != actual.cend())
        result += '+' + *aIt + '\n';
    return result;
}

// Remove XML tags by position (with potentially leading blanks  and including the trailing '\n').
static void removeTag(qsizetype &pos, qsizetype endPos, QByteArray *ui)
{
    if (pos > 0) {
        if (const auto lineStart = ui->lastIndexOf('\n', pos); lineStart != -1)
            pos = lineStart + 1;
    }
    if (endPos < ui->size() && ui->at(endPos) == '\n')
        ++endPos;
    ui->remove(pos, endPos - pos);
}

static void removeTag(QByteArrayView beginTag, QByteArrayView endTag, QByteArray *ui)
{
    if (auto pos = ui->indexOf(beginTag); pos != -1) {
        const auto endPos = ui->indexOf(endTag, pos + beginTag.size());
        if (endPos != -1)
            removeTag(pos, endPos + endTag.size(), ui);
    }
}

// Remove a number of properties that depend on platform/style
static QByteArray prepareActualUi(QByteArray ui)
{
    static constexpr QByteArrayView beginProperty = "<property name=\"";
    static constexpr QByteArrayView endProperty = "</property>";

    static const std::unordered_set<QByteArrayView> skipProperties = {
        "autoFillBackground", "backgroundBrush", "cursor", "font",
        "foregroundBrush", "geometry", "locale", "mouseTracking",
        "palette", "pos","size"
    };

    for (auto pos = ui.indexOf(beginProperty); pos != -1; pos = ui.indexOf(beginProperty, pos)) {
        const auto namePos = pos + beginProperty.size();
        const auto nameEndPos = ui.indexOf('"', namePos);
        assert(nameEndPos != -1);
        const auto name = QByteArrayView(ui).sliced(namePos, nameEndPos - namePos);
        if (skipProperties.count(name) > 0) {
            const auto endPos = ui.indexOf(endProperty, nameEndPos);
            assert(endPos != -1);
            removeTag(pos, endPos + endProperty.size(), &ui);
        } else {
            pos = nameEndPos;
        }
    }
    return ui;
}

static QByteArray saveForm(QWidget *w)
{
    QByteArray result;
    QBuffer buf(&result);
    buf.open(QIODevice::WriteOnly);
    QFormBuilder().save(&buf, w);
    return prepareActualUi(result);
}

// Test data for QTBUG-151846: Verify that unset options like QGraphicsView::optimizationFlags
// do not cause an empty <set/> to be written.
static std::unique_ptr<QWidget> createGraphicsView()
{
    auto result = std::make_unique<QGraphicsView>();
    result->setRenderHint(QPainter::SmoothPixmapTransform); // set some flag/enums
    result->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    return result;
}

// Test QFormBuilder by creating widgets and comparing the output of save() against a baseline.
class tst_QFormBuilder : public QObject
{
    Q_OBJECT
public:
    static void initMain() { qputenv("QT_QPA_PLATFORM", qpaPlatform);}

private slots:
    void initTestCase();
    void save_data();
    void save();

private:
    QString m_baseline;
};

void tst_QFormBuilder::initTestCase()
{
    m_baseline = QFINDTESTDATA("baseline");
    QVERIFY2(!m_baseline.isEmpty(), "Could not find 'baseline'.");
}

void tst_QFormBuilder::save_data()
{
    QTest::addColumn<CreationFunction>("creationFunction");
    QTest::addColumn<QString>("fileName");

    QTest::newRow("qgraphicsview") << &createGraphicsView << u"qgraphicsview.ui"_s;
}

void tst_QFormBuilder::save()
{
    QFETCH(CreationFunction, creationFunction);
    QFETCH(QString, fileName);

    static const bool writeActual = qEnvironmentVariableIsSet(writeBaselineEnvVar);

    auto widget = creationFunction();
    const QByteArray actual = saveForm(widget.get());

    if (writeActual) {
        QFile actualFile(m_baseline + u'/' + fileName + ".baseline"_L1);
        if (actualFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            actualFile.write(actual);
            qInfo(" Wrote %s (remember to add license header).",
                  qPrintable(QDir::toNativeSeparators(actualFile.fileName())));
        } else {
            qWarning("%s", qPrintable(msgCannotOpen(actualFile)));
        }
    }

    QFile file(m_baseline + u'/' + fileName);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(msgCannotOpen(file)));
    QByteArray expected = file.readAll();
    removeTag("<comment>"_ba, "</comment>"_ba, &expected); // Remove license comment
    file.close();

    QVERIFY2(actual == expected, msgDiff(actual, expected).constData());
}

QTEST_MAIN(tst_QFormBuilder)

#include "tst_qformbuilder.moc"
