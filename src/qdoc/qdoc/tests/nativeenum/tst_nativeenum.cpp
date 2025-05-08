// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdoc/nativeenum.h"

#include <QtTest/QtTest>

QT_BEGIN_NAMESPACE

class StubEnumNode : public NativeEnumInterface
{
public:
    explicit StubEnumNode() { m_nativeEnum.setPrefix(QStringLiteral("dummy")); }

    const NativeEnum *nativeEnum() const override { return &m_nativeEnum; }
    NativeEnum *nativeEnum() override { return &m_nativeEnum; }

private:
    NativeEnum m_nativeEnum;
};

QT_END_NAMESPACE

class tst_NativeEnum : public QObject
{
    Q_OBJECT
private slots:
    void basic();
};

void tst_NativeEnum::basic()
{
    QT_PREPEND_NAMESPACE(StubEnumNode) n;
    QT_PREPEND_NAMESPACE(StubEnumNode) *ptr = &n;

    auto *ne_if = dynamic_cast<NativeEnumInterface *>(ptr);
    QVERIFY(ne_if);
    QVERIFY(ne_if->nativeEnum()->enumNode() == nullptr);
    QCOMPARE(ne_if->nativeEnum()->prefix(), QStringLiteral("dummy"));
}

QTEST_APPLESS_MAIN(tst_NativeEnum)
#include "tst_nativeenum.moc"
