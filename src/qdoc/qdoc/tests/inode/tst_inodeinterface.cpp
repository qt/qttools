// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdoc/genustypes.h"
#include "qdoc/inode.h"

#include <QtTest/QtTest>

QT_BEGIN_NAMESPACE

class StubNode : public INode
{
public:
    explicit StubNode(QString name) : m_name(std::move(name)) {}

    [[nodiscard]] Genus genus() const override { return Genus::DontCare; }
    [[nodiscard]] NodeType nodeType() const override { return NodeType::NoType; }

    [[nodiscard]] const QString &name() const override { return m_name; }
    [[nodiscard]] QString fullName() const override
    {
        // Trivial implementation for compile-time interface check.
        return m_name;
    }

private:
    QString m_name;
};

QT_END_NAMESPACE

class tst_INodeInterface : public QObject
{
    Q_OBJECT
private slots:
    void basic();
};

void tst_INodeInterface::basic()
{
    QT_PREPEND_NAMESPACE(StubNode) n(QStringLiteral("dummy"));
    QT_PREPEND_NAMESPACE(INode)   *ptr = &n;

    QCOMPARE(ptr->name(),     QStringLiteral("dummy"));
    QCOMPARE(ptr->fullName(), QStringLiteral("dummy"));
}

QTEST_APPLESS_MAIN(tst_INodeInterface)
#include "tst_inodeinterface.moc"
