// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtDesigner/private/qdesigner_utils_p.h>

#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qlabel.h>

#include <QtCore/qmetaobject.h>
#include <QtCore/qstring.h>

#include <QTest>

using namespace Qt::StringLiterals;
using namespace qdesigner_internal;

static DesignerMetaFlags flagsFor(const QMetaObject *mo, const char *property)
{
    const QMetaEnum me = mo->property(mo->indexOfProperty(property)).enumerator();
    DesignerMetaFlags result(QString::fromLatin1(me.enumName()),
                             QString::fromLatin1(me.scope()), u"::"_s);
    for (int i = 0, count = me.keyCount(); i < count; ++i)
        result.addKey(uint(me.value(i)), QString::fromLatin1(me.key(i)));
    return result;
}

static DesignerMetaFlags alignmentFlags()
{
    return flagsFor(&QLabel::staticMetaObject, "alignment");
}

static DesignerMetaFlags dockWidgetAreaFlags()
{
    return flagsFor(&QDockWidget::staticMetaObject, "allowedAreas");
}

class tst_QDesignerMetaFlags : public QObject
{
    Q_OBJECT

private slots:
    void toString_data();
    void toString();
    void parseExistingForms_data();
    void parseExistingForms();
    void aggregateAliasKeepsItsName_data();
    void aggregateAliasKeepsItsName();
};

void tst_QDesignerMetaFlags::toString_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<int>("mode");
    QTest::addColumn<QString>("expected");

    const int full = DesignerMetaFlags::FullyQualified;
    const int qual = DesignerMetaFlags::Qualified;

    QTest::newRow("left|top")
        << int(Qt::AlignLeft | Qt::AlignTop) << full
        << u"Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop"_s;
    QTest::newRow("right|vcenter")
        << int(Qt::AlignRight | Qt::AlignVCenter) << full
        << u"Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter"_s;

    QTest::newRow("left")
        << int(Qt::AlignLeft) << full << u"Qt::AlignmentFlag::AlignLeft"_s;
    QTest::newRow("right")
        << int(Qt::AlignRight) << full << u"Qt::AlignmentFlag::AlignRight"_s;

    QTest::newRow("left|top, qualified")
        << int(Qt::AlignLeft | Qt::AlignTop) << qual
        << u"Qt::AlignLeft|Qt::AlignTop"_s;
    QTest::newRow("left, qualified")
        << int(Qt::AlignLeft) << qual << u"Qt::AlignLeft"_s;

    QTest::newRow("hcenter|top")
        << int(Qt::AlignHCenter | Qt::AlignTop) << full
        << u"Qt::AlignmentFlag::AlignHCenter|Qt::AlignmentFlag::AlignTop"_s;
    QTest::newRow("center")
        << int(Qt::AlignCenter) << full << u"Qt::AlignmentFlag::AlignCenter"_s;
}

void tst_QDesignerMetaFlags::toString()
{
    QFETCH(int, value);
    QFETCH(int, mode);
    QFETCH(QString, expected);

    const DesignerMetaFlags flags = alignmentFlags();
    const auto sm = static_cast<DesignerMetaFlags::SerializationMode>(mode);
    const QString actual = flags.toString(value, sm);

    bool ok = false;
    QCOMPARE(int(flags.parseFlags(actual, &ok)), value);
    QVERIFY(ok);

    QCOMPARE(actual, expected);
}

void tst_QDesignerMetaFlags::parseExistingForms_data()
{
    QTest::addColumn<QString>("serialized");
    QTest::addColumn<int>("expected");

    const int leftTop = Qt::AlignLeft | Qt::AlignTop;
    const int rightVCenter = Qt::AlignRight | Qt::AlignVCenter;

    QTest::newRow("designer 6.7+, aliases")
        << u"Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop"_s
        << leftTop;
    QTest::newRow("designer pre-6.7, aliases")
        << u"Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop"_s << leftTop;
    QTest::newRow("unqualified, aliases")
        << u"AlignLeading|AlignLeft|AlignTop"_s << leftTop;
    QTest::newRow("designer 6.7+, trailing alias")
        << u"Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter"_s
        << rightVCenter;
    QTest::newRow("designer pre-6.7, trailing alias")
        << u"Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter"_s << rightVCenter;

    QTest::newRow("formbuilder, alias only")
        << u"Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignTop"_s << leftTop;
    QTest::newRow("formbuilder pre-5.5, canonical only")
        << u"Qt::AlignRight|Qt::AlignVCenter"_s << rightVCenter;

    QTest::newRow("full mask")
        << u"Qt::AlignAbsolute|Qt::AlignBottom|Qt::AlignCenter|Qt::AlignHCenter"
           "|Qt::AlignHorizontal_Mask|Qt::AlignJustify|Qt::AlignLeading|Qt::AlignLeft"
           "|Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing|Qt::AlignVCenter"
           "|Qt::AlignVertical_Mask"_s
        << int(Qt::AlignAbsolute | Qt::AlignBottom | Qt::AlignCenter | Qt::AlignHCenter
               | Qt::AlignHorizontal_Mask | Qt::AlignJustify | Qt::AlignLeading | Qt::AlignLeft
               | Qt::AlignRight | Qt::AlignTop | Qt::AlignTrailing | Qt::AlignVCenter
               | Qt::AlignVertical_Mask);

    QTest::newRow("single alias")
        << u"Qt::AlignmentFlag::AlignLeading"_s << int(Qt::AlignLeft);
    QTest::newRow("no alias involved")
        << u"Qt::AlignmentFlag::AlignHCenter|Qt::AlignmentFlag::AlignTop"_s
        << int(Qt::AlignHCenter | Qt::AlignTop);
}

void tst_QDesignerMetaFlags::parseExistingForms()
{
    QFETCH(QString, serialized);
    QFETCH(int, expected);

    const DesignerMetaFlags flags = alignmentFlags();

    bool ok = false;
    QCOMPARE(int(flags.parseFlags(serialized, &ok)), expected);
    QVERIFY(ok);

    for (auto sm : {DesignerMetaFlags::FullyQualified, DesignerMetaFlags::Qualified}) {
        const QString rewritten = flags.toString(expected, sm);
        bool rewrittenOk = false;
        QCOMPARE(int(flags.parseFlags(rewritten, &rewrittenOk)), expected);
        QVERIFY(rewrittenOk);
    }
}

void tst_QDesignerMetaFlags::aggregateAliasKeepsItsName_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("expected");

    QTest::newRow("all areas")
        << int(Qt::AllDockWidgetAreas)
        << u"Qt::DockWidgetArea::AllDockWidgetAreas"_s;
    QTest::newRow("left|right")
        << int(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea)
        << u"Qt::DockWidgetArea::LeftDockWidgetArea|Qt::DockWidgetArea::RightDockWidgetArea"_s;
    QTest::newRow("single area")
        << int(Qt::TopDockWidgetArea) << u"Qt::DockWidgetArea::TopDockWidgetArea"_s;
}

void tst_QDesignerMetaFlags::aggregateAliasKeepsItsName()
{
    QFETCH(int, value);
    QFETCH(QString, expected);

    const DesignerMetaFlags flags = dockWidgetAreaFlags();
    QCOMPARE(flags.toString(value, DesignerMetaFlags::FullyQualified), expected);

    bool ok = false;
    QCOMPARE(int(flags.parseFlags(expected, &ok)), value);
    QVERIFY(ok);
}

QTEST_APPLESS_MAIN(tst_QDesignerMetaFlags)

#include "tst_qdesignermetaflags.moc"
