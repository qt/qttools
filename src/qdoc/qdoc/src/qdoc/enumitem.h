// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ENUMITEM_H
#define ENUMITEM_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

#include <utility>

QT_BEGIN_NAMESPACE

class EnumItem
{
public:
    EnumItem() = default;
    EnumItem(QString name, QString value, QString since = QString())
            : m_name(std::move(name)),
              m_value(std::move(value)),
              m_since(std::move(since)) {}

    [[nodiscard]] const QString &name() const { return m_name; }
    [[nodiscard]] const QString &value() const { return m_value; }
    [[nodiscard]] const QString &since() const { return m_since; }
    void setSince(const QString &since) { m_since = since; }

private:
    QString m_name {};
    QString m_value {};
    QString m_since {};
};

QT_END_NAMESPACE

#endif // ENUMITEM_H
