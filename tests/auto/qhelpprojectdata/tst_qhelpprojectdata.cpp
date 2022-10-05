// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include <QtTest/QtTest>

#include <QtCore/QFileInfo>

#include "../../../src/assistant/qhelpgenerator/qhelpprojectdata_p.h"

class tst_QHelpProjectData : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void readData();
    void namespaceName();
    void virtualFolder();
    void customFilters();
    void filterSections();
    void metaData();
    void rootPath();

private:
    QString m_inputFile;
};

void tst_QHelpProjectData::initTestCase()
{
    const QString path = QLatin1String(SRCDIR);
    m_inputFile = path + QLatin1String("/data/test.qhp");
}

void tst_QHelpProjectData::readData()
{
    QHelpProjectData data;
    if (!data.readData(m_inputFile))
        QFAIL("Cannot parse qhp file!");
}

void tst_QHelpProjectData::namespaceName()
{
    QHelpProjectData data;
    if (!data.readData(m_inputFile))
        QFAIL("Cannot read qhp file!");
    QCOMPARE(data.namespaceName(), QString("trolltech.com.1.0.0.test"));
}

void tst_QHelpProjectData::virtualFolder()
{
    QHelpProjectData data;
    if (!data.readData(m_inputFile))
        QFAIL("Cannot read qhp file!");
    QCOMPARE(data.virtualFolder(), QString("testFolder"));
}

void tst_QHelpProjectData::customFilters()
{
    QHelpProjectData data;
    if (!data.readData(m_inputFile))
        QFAIL("Cannot read qhp file!");

    const QList<QHelpDataCustomFilter> filters = data.customFilters();
    QCOMPARE(filters.size(), 2);

    for (const QHelpDataCustomFilter &f : filters) {
        if (f.name == QLatin1String("Custom Filter 1")) {
            for (const QString &id : f.filterAttributes) {
                if (id != QLatin1String("test")
                    && id != QLatin1String("filter1"))
                    QFAIL("Wrong filter attribute!");
            }
        } else if (f.name == QLatin1String("Custom Filter 2")) {
            for (const QString &id : f.filterAttributes) {
                if (id != QLatin1String("test")
                    && id != QLatin1String("filter2"))
                    QFAIL("Wrong filter attribute!");
            }
        } else {
            QFAIL("Unexpected filter name!");
        }
    }
}

void tst_QHelpProjectData::filterSections()
{
    QHelpProjectData data;
    if (!data.readData(m_inputFile))
        QFAIL("Cannot read qhp file!");

    const QList<QHelpDataFilterSection> sections = data.filterSections();
    QCOMPARE(sections.size(), 2);

    for (const QHelpDataFilterSection &s : sections) {
        if (s.filterAttributes().contains("filter1")) {
            const auto indices = s.indices();
            QCOMPARE(indices.size(), 5);
            for (const QHelpDataIndexItem &idx : indices) {
                if (idx.name == QLatin1String("foo")) {
                    QCOMPARE(idx.identifier, QString("Test::foo"));
                } else if (idx.name == QLatin1String("bar")) {
                    QCOMPARE(idx.reference, QString("test.html#bar"));
                } else if (idx.name == QLatin1String("bla")) {
                    QCOMPARE(idx.identifier, QString("Test::bla"));
                } else if (idx.name == QLatin1String("einstein")) {
                    QCOMPARE(idx.reference, QString("people.html#einstein"));
                } else if (idx.name == QLatin1String("newton")) {
                    QCOMPARE(idx.identifier, QString("People::newton"));
                } else {
                    QFAIL("Unexpected index!");
                }
            }
            QCOMPARE(s.contents().size(), 1);
            QCOMPARE(s.contents().first()->children().size(), 5);
        } else if (s.filterAttributes().contains("filter2")) {
            QCOMPARE(s.contents().size(), 1);
            const QStringList lst = {
                "cars.html",
                "classic.css",
                "fancy.html",
            };
            auto files = s.files();
            std::sort(files.begin(), files.end());
            QCOMPARE(files, lst);
        } else {
            QFAIL("Unexpected filter attribute!");
        }
    }
}

void tst_QHelpProjectData::metaData()
{
    QHelpProjectData data;
    if (!data.readData(m_inputFile))
        QFAIL("Cannot read qhp file!");

    QCOMPARE(data.metaData().size(), 2);
    QCOMPARE(data.metaData().value("author").toString(),
        QString("Digia Plc and/or its subsidiary(-ies)"));
}

void tst_QHelpProjectData::rootPath()
{
    QHelpProjectData data;
    if (!data.readData(m_inputFile))
        QFAIL("Cannot read qhp file!");

    QFileInfo fi(m_inputFile);
    QCOMPARE(data.rootPath(), fi.absolutePath());
}

QTEST_MAIN(tst_QHelpProjectData)
#include "tst_qhelpprojectdata.moc"
