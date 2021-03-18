/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "projectdescriptionreader.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qfile.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qset.h>

#include <algorithm>
#include <functional>

using std::placeholders::_1;

class FMT {
    Q_DECLARE_TR_FUNCTIONS(Linguist)
};

class Validator
{
public:
    Validator(QString *errorString)
        : m_errorString(errorString)
    {
    }

    bool isValidProjectDescription(const QJsonArray &projects)
    {
        return std::all_of(projects.begin(), projects.end(),
                           std::bind(&Validator::isValidProjectObject, this, _1));
    }

private:
    bool isValidProject(const QJsonObject &project)
    {
        static const QSet<QString> requiredKeys = {
            QStringLiteral("projectFile"),
        };
        static const QSet<QString> allowedKeys
                = QSet<QString>(requiredKeys)
                    << QStringLiteral("codec")
                    << QStringLiteral("excluded")
                    << QStringLiteral("includePaths")
                    << QStringLiteral("sources")
                    << QStringLiteral("subProjects")
                    << QStringLiteral("translations");
        QSet<QString> actualKeys;
        for (auto it = project.constBegin(), end = project.constEnd(); it != end; ++it)
            actualKeys.insert(it.key());
        const QSet<QString> missingKeys = requiredKeys - actualKeys;
        if (!missingKeys.isEmpty()) {
            *m_errorString = FMT::tr("Missing keys in project description: %1.").arg(
                    missingKeys.values().join(QLatin1String(", ")));
            return false;
        }
        const QSet<QString> unexpected = actualKeys - allowedKeys;
        if (!unexpected.isEmpty()) {
            *m_errorString = FMT::tr("Unexpected keys in project %1: %2").arg(
                    project.value(QStringLiteral("projectFile")).toString(),
                    unexpected.values().join(QLatin1String(", ")));
            return false;
        }
        return isValidProjectDescription(project.value(QStringLiteral("subProjects")).toArray());
    }

    bool isValidProjectObject(const QJsonValue &v)
    {
        if (!v.isObject()) {
            *m_errorString = FMT::tr("JSON object expected.");
            return false;
        }
        return isValidProject(v.toObject());
    }

    QString *m_errorString;
};

static QJsonArray readRawProjectDescription(const QString &filePath, QString *errorString)
{
    errorString->clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        *errorString = FMT::tr("Cannot open project description file '%1'.\n")
            .arg(filePath);
        return {};
    }
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (doc.isNull()) {
        *errorString = FMT::tr("%1 in %2 at offset %3.\n")
            .arg(parseError.errorString(), filePath)
            .arg(parseError.offset);
        return {};
    }
    QJsonArray result = doc.isArray() ? doc.array() : QJsonArray{doc.object()};
    Validator validator(errorString);
    if (!validator.isValidProjectDescription(result))
        return {};
    return result;
}

class ProjectConverter
{
public:
    ProjectConverter(QString *errorString)
        : m_errorString(*errorString)
    {
    }

    Projects convertProjects(const QJsonArray &rawProjects)
    {
        Projects result;
        result.reserve(rawProjects.size());
        for (const QJsonValue &rawProject : rawProjects) {
            Project project = convertProject(rawProject);
            if (!m_errorString.isEmpty())
                break;
            result.push_back(std::move(project));
        }
        return result;
    }

private:
    Project convertProject(const QJsonValue &v)
    {
        if (!v.isObject())
            return {};
        Project result;
        QJsonObject obj = v.toObject();
        result.filePath = stringValue(obj, QLatin1String("projectFile"));
        result.codec = stringValue(obj, QLatin1String("codec"));
        result.excluded = stringListValue(obj, QLatin1String("excluded"));
        result.includePaths = stringListValue(obj, QLatin1String("includePaths"));
        result.sources = stringListValue(obj, QLatin1String("sources"));
        if (obj.contains(QLatin1String("translations")))
            result.translations.reset(new QStringList(stringListValue(obj, QLatin1String("translations"))));
        result.subProjects = convertProjects(obj.value(QLatin1String("subProjects")).toArray());
        return result;
    }

    bool checkType(const QJsonValue &v, QJsonValue::Type t, const QString &key)
    {
        if (v.type() == t)
            return true;
        m_errorString = FMT::tr("Key %1 should be %2 but is %3.").arg(key, jsonTypeName(t),
                                                                      jsonTypeName(v.type()));
        return false;
    }

    static QString jsonTypeName(QJsonValue::Type t)
    {
        // ### If QJsonValue::Type was declared with Q_ENUM we could just query QMetaEnum.
        switch (t) {
        case QJsonValue::Null:
            return QStringLiteral("null");
        case QJsonValue::Bool:
            return QStringLiteral("bool");
        case QJsonValue::Double:
            return QStringLiteral("double");
        case QJsonValue::String:
            return QStringLiteral("string");
        case QJsonValue::Array:
            return QStringLiteral("array");
        case QJsonValue::Object:
            return QStringLiteral("object");
        case QJsonValue::Undefined:
            return QStringLiteral("undefined");
        }
        return QStringLiteral("unknown");
    }

    QString stringValue(const QJsonObject &obj, const QString &key)
    {
        if (!m_errorString.isEmpty())
            return {};
        QJsonValue v = obj.value(key);
        if (v.isUndefined())
            return {};
        if (!checkType(v, QJsonValue::String, key))
            return {};
        return v.toString();
    }

    QStringList stringListValue(const QJsonObject &obj, const QString &key)
    {
        if (!m_errorString.isEmpty())
            return {};
        QJsonValue v = obj.value(key);
        if (v.isUndefined())
            return {};
        if (!checkType(v, QJsonValue::Array, key))
            return {};
        return toStringList(v, key);
    }

    QStringList toStringList(const QJsonValue &v, const QString &key)
    {
        QStringList result;
        const QJsonArray a = v.toArray();
        result.reserve(a.count());
        for (const QJsonValue &v : a) {
            if (!v.isString()) {
                m_errorString = FMT::tr("Unexpected type %1 in string array in key %2.")
                        .arg(jsonTypeName(v.type()), key);
                return {};
            }
            result.append(v.toString());
        }
        return result;
    }

    QString &m_errorString;
};

Projects readProjectDescription(const QString &filePath, QString *errorString)
{
    const QJsonArray rawProjects = readRawProjectDescription(filePath, errorString);
    if (!errorString->isEmpty())
        return {};
    ProjectConverter converter(errorString);
    Projects result = converter.convertProjects(rawProjects);
    if (!errorString->isEmpty())
        return {};
    return result;
}
