// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TESTACTION_H
#define TESTACTION_H

#include <QtCore/QObject>

class TestAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY changed)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY changed)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY changed)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable NOTIFY singlePropertySignal)

public:
    explicit TestAction(QObject *parent = nullptr);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isVisible() const;
    void setVisible(bool visible);

    QString text() const;
    void setText(const QString &text);

    bool isCheckable() const;
    void setCheckable(bool checkable);

signals:
    void changed(); // This signal notifies for multiple properties (enabled, visible, text)
    void singlePropertySignal(); // This signal notifies for a single property (checkable)

private:
    bool m_enabled = true;
    bool m_visible = true;
    QString m_text;
    bool m_checkable = false;
};

#endif // TESTACTION_H

