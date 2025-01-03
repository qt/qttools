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
    EnumItem(QString name, QString value) : m_name(std::move(name)), m_value(std::move(value)) { }

    [[nodiscard]] const QString &name() const { return m_name; }
    [[nodiscard]] const QString &value() const { return m_value; }

private:
    QString m_name {};
    QString m_value {};
};

QT_END_NAMESPACE

#endif // ENUMITEM_H
