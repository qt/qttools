// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtHelp/QHelpEngineCore>
#include <QtQmlLS/private/qqmllshelputils_p.h>

// TODO (Qt 7.0)
// Remove this test as well as the plugin from QtTools when the QtHelp lib
// is split into QtHelpCore and QtHelp. Then QmlLS can depend only on QtHelpCore.
class tst_HelpEnginePlugin : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void documentationForItem_data();
    void documentationForItem();

private:
    HelpManager helpManager;
};

void tst_HelpEnginePlugin::initTestCase()
{
    helpManager.setDocumentationRootPath(QStringLiteral(DATADIR));
}

void tst_HelpEnginePlugin::documentationForItem_data()
{
    using namespace QQmlJS::Dom;
    QTest::addColumn<DomItem>("fileItem");
    QTest::addColumn<QLspSpecification::Position>("hoveredPosition");
    QTest::addColumn<QByteArray>("expectedDocumentation");
    const auto fileObject = [](const QString &filePath) {
        QFile f(filePath);
        DomItem file;
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return file;
        QString code = f.readAll();
        DomCreationOption option = DomCreationOption::Extended;
        QStringList dirs = { QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath) };
        auto envPtr = DomEnvironment::create(
                dirs,
                QQmlJS::Dom::DomEnvironment::Option::SingleThreaded
                        | QQmlJS::Dom::DomEnvironment::Option::NoDependencies,
                option);
        envPtr->loadBuiltins();
        envPtr->loadFile(FileToLoad::fromMemory(envPtr, filePath, code),
                         [&file](Path, const DomItem &, const DomItem &newIt) {
                             file = newIt.fileObject();
                         });
        envPtr->loadPendingDependencies();
        return file;
    };
    const auto filePath = QStringLiteral(DATADIR) + "/elements.qml"_L1;
    const auto fileItem = fileObject(filePath);
    QTest::addRow("qmlObjectHoveredAtBegin") << fileItem << QLspSpecification::Position{ 7, 4 }
                              << QByteArray("Encapsulates a QML component definition.");
    QTest::addRow("qmlObjectHoveredAtEnd") << fileItem << QLspSpecification::Position{ 7, 13 }
                              << QByteArray("Encapsulates a QML component definition.");
    QTest::addRow("propertyBinding") << fileItem << QLspSpecification::Position{ 15, 15 }
                              << QByteArray("Sets the interval between triggers, in milliseconds.");
    QTest::addRow("propertyTypeIsSameAsName") << fileItem << QLspSpecification::Position{10, 35 }
                              << QByteArray("The component URL. This is the URL that was used to construct the component.");
    QTest::addRow("method") << fileItem << QLspSpecification::Position{ 16, 26 }
                              << QByteArray("Restarts the timer");
}

void tst_HelpEnginePlugin::documentationForItem()
{
    using namespace QQmlJS::Dom;
    QFETCH(DomItem, fileItem);
    QFETCH(QLspSpecification::Position, hoveredPosition);
    QFETCH(QByteArray, expectedDocumentation);
    HelpManager helpManager;
    helpManager.setDocumentationRootPath(QStringLiteral(DATADIR));
    const auto actual = helpManager.documentationForItem(fileItem, hoveredPosition);
    QVERIFY(actual.has_value());
    QCOMPARE(actual.value(), expectedDocumentation);
}

QTEST_MAIN(tst_HelpEnginePlugin)
#include "tst_helpengineplugin.moc"
