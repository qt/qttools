// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PARAMETER_H
#define PARAMETER_H

#include <QtCore/qstring.h>

#include <utility>

QT_BEGIN_NAMESPACE

class Parameter
{
public:
    Parameter() = default;
    explicit Parameter(QString type, QString name = QString(), QString defaultValue = QString())
        : m_type(std::move(type)), m_name(std::move(name)), m_defaultValue(std::move(defaultValue))
    {
    }

    void setName(const QString &name) { m_name = name; }
    [[nodiscard]] bool hasType() const { return !m_type.isEmpty(); }
    [[nodiscard]] const QString &type() const { return m_type; }
    [[nodiscard]] const QString &name() const { return m_name; }
    [[nodiscard]] const QString &defaultValue() const { return m_defaultValue; }
    void setDefaultValue(const QString &t) { m_defaultValue = t; }

    void set(const QString &type, const QString &name, const QString &defaultValue = QString())
    {
        m_type = type;
        m_name = name;
        m_defaultValue = defaultValue;
    }

    [[nodiscard]] QString signature(bool includeValue = false) const;
    [[nodiscard]] qsizetype nameInsertionPoint() const;

    [[nodiscard]] const QString &canonicalType() const { return m_canonicalType; }
    void setCanonicalType(const QString &t) { m_canonicalType = t; }

private:
    QString m_canonicalType {};
    QString m_type {};
    QString m_name {};
    QString m_defaultValue {};
};

typedef QList<Parameter> ParameterVector;

QT_END_NAMESPACE

#endif // PARAMETER_H

