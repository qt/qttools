/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QtCore/qregexp.h>
#include <QtCore/qset.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class Location;
class Tokenizer;
class CodeChunk;

class Parameter
{
public:
    Parameter() {}
    Parameter(const QString &type, const QString &name = QString(),
              const QString &defaultValue = QString())
        : type_(type), name_(name), defaultValue_(defaultValue)
    {
    }

    void setName(const QString &name) { name_ = name; }
    bool hasType() const { return !type_.isEmpty(); }
    const QString &type() const { return type_; }
    const QString &name() const { return name_; }
    const QString &defaultValue() const { return defaultValue_; }
    void setDefaultValue(const QString &t) { defaultValue_ = t; }

    void set(const QString &type, const QString &name, const QString &defaultValue = QString())
    {
        type_ = type;
        name_ = name;
        defaultValue_ = defaultValue;
    }

    QString signature(bool includeValue = false) const;

public:
    QString type_;
    QString name_;
    QString defaultValue_;
};

typedef QVector<Parameter> ParameterVector;

class Parameters
{
public:
    Parameters();
    Parameters(const QString &signature);

    void clear()
    {
        parameters_.clear();
        privateSignal_ = false;
        valid_ = true;
    }
    const ParameterVector &parameters() const { return parameters_; }
    bool isPrivateSignal() const { return privateSignal_; }
    bool isEmpty() const { return parameters_.isEmpty(); }
    bool isValid() const { return valid_; }
    int count() const { return parameters_.size(); }
    void reserve(int count) { parameters_.reserve(count); }
    const Parameter &at(int i) const { return parameters_.at(i); }
    const Parameter &last() const { return parameters_.last(); }
    inline Parameter &operator[](int index) { return parameters_[index]; }
    void append(const QString &type, const QString &name, const QString &value);
    void append(const QString &type, const QString &name) { append(type, name, QString()); }
    void append(const QString &type) { append(type, QString(), QString()); }
    void pop_back() { parameters_.pop_back(); }
    void setPrivateSignal() { privateSignal_ = true; }
    QString signature(bool includeValues = false) const;
    QString rawSignature(bool names = false, bool values = false) const;
    void set(const QString &signature);
    QSet<QString> getNames() const;
    QString generateTypeList() const;
    QString generateTypeAndNameList() const;
    bool match(const Parameters &parameters) const;

private:
    void readToken();
    QString lexeme();
    QString previousLexeme();
    bool match(int target);
    void matchTemplateAngles(CodeChunk &type);
    bool matchTypeAndName(CodeChunk &type, QString &name, bool qProp = false);
    bool matchParameter();
    bool parse(const QString &signature);

private:
    static QRegExp varComment_;

    bool valid_;
    bool privateSignal_;
    int tok_;
    Tokenizer *tokenizer_;
    ParameterVector parameters_;
};

QT_END_NAMESPACE

#endif
