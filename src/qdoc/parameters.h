// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QtCore/qlist.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qset.h>

#include <utility>

QT_BEGIN_NAMESPACE

class Location;
class Tokenizer;
class CodeChunk;

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

    [[nodiscard]] const QString &canonicalType() const { return m_canonicalType; }
    void setCanonicalType(const QString &t) { m_canonicalType = t; }

public:
    QString m_canonicalType {};
    QString m_type {};
    QString m_name {};
    QString m_defaultValue {};
};

typedef QList<Parameter> ParameterVector;

class Parameters
{
public:
    Parameters();
    Parameters(const QString &signature); // TODO: Making this explicit breaks QDoc

    void clear()
    {
        m_parameters.clear();
        m_privateSignal = false;
        m_valid = true;
    }
    [[nodiscard]] const ParameterVector &parameters() const { return m_parameters; }
    [[nodiscard]] bool isPrivateSignal() const { return m_privateSignal; }
    [[nodiscard]] bool isEmpty() const { return m_parameters.isEmpty(); }
    [[nodiscard]] bool isValid() const { return m_valid; }
    [[nodiscard]] int count() const { return m_parameters.size(); }
    void reserve(int count) { m_parameters.reserve(count); }
    [[nodiscard]] const Parameter &at(int i) const { return m_parameters.at(i); }
    Parameter &last() { return m_parameters.last(); }
    [[nodiscard]] const Parameter &last() const { return m_parameters.last(); }
    inline Parameter &operator[](int index) { return m_parameters[index]; }
    void append(const QString &type, const QString &name, const QString &value);
    void append(const QString &type, const QString &name) { append(type, name, QString()); }
    void append(const QString &type) { append(type, QString(), QString()); }
    void pop_back() { m_parameters.pop_back(); }
    void setPrivateSignal() { m_privateSignal = true; }
    [[nodiscard]] QString signature(bool includeValues = false) const;
    [[nodiscard]] QString rawSignature(bool names = false, bool values = false) const;
    void set(const QString &signature);
    [[nodiscard]] QSet<QString> getNames() const;
    [[nodiscard]] QString generateTypeList() const;
    [[nodiscard]] QString generateTypeAndNameList() const;
    [[nodiscard]] bool match(const Parameters &parameters) const;

private:
    void readToken();
    QString lexeme();
    QString previousLexeme();
    bool match(int target);
    void matchTemplateAngles(CodeChunk &type);
    bool matchTypeAndName(CodeChunk &type, QString &name);
    bool matchParameter();
    bool parse(const QString &signature);

private:
    static QRegularExpression s_varComment;

    bool m_valid {};
    bool m_privateSignal {};
    int m_tok {};
    Tokenizer *m_tokenizer { nullptr };
    ParameterVector m_parameters;
};

QT_END_NAMESPACE

#endif
