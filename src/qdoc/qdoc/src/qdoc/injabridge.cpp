// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "injabridge.h"

QT_BEGIN_NAMESPACE

/*!
    \class InjaBridge
    \brief Adapter for converting Qt JSON types to Inja template engine format.

    InjaBridge provides static methods to convert between Qt's native JSON types
    (QJsonObject, QJsonArray, QJsonValue) and nlohmann::json, which is the data
    format expected by the Inja template engine.

    This adapter allows QDoc to maintain its Qt-native API while leveraging
    Inja for template-based documentation generation. All JSON data in QDoc's
    intermediate representation (IR) uses Qt types, and InjaBridge handles the
    conversion when rendering templates.

    \note All numbers in QJsonValue are stored as doubles. When converted to
    nlohmann::json, integers are rendered with decimal points (e.g., 30 becomes
    "30.0" in template output). This is expected behavior.

    \note Inja and nlohmann::json may report template or data errors. QDoc is
    built with exceptions disabled (\c{-fno-exceptions}), so such errors are
    treated as fatal and will terminate the process. A custom \c INJA_THROW
    override in the header ensures that error details (including source
    location) are logged via \c qFatal() before termination, rather than
    calling \c std::abort() silently.

    \sa QJsonObject, QJsonArray, QJsonValue
*/

/*!
    \brief Converts a QJsonValue, \a value, to nlohmann::json.

    Handles all QJsonValue types: Null, Bool, Double, String, Array, Object,
    and Undefined. Undefined values are treated as null.

    Returns the equivalent nlohmann::json representation.
*/
nlohmann::json InjaBridge::toInjaJson(const QJsonValue &value)
{
    switch (value.type()) {
    case QJsonValue::Null:
        return nullptr;
    case QJsonValue::Bool:
        return value.toBool();
    case QJsonValue::Double:
        return value.toDouble();
    case QJsonValue::String:
        return value.toString().toUtf8().toStdString();
    case QJsonValue::Array:
        return toInjaJson(value.toArray());
    case QJsonValue::Object:
        return toInjaJson(value.toObject());
    case QJsonValue::Undefined:
        return nullptr;
    }
    return nullptr;
}

/*!
    \brief Converts a QJsonObject, \a obj, to nlohmann::json.

    Recursively converts all values in the object, preserving the key-value
    structure. Nested objects and arrays are handled correctly.

    Returns the equivalent nlohmann::json object.
*/
nlohmann::json InjaBridge::toInjaJson(const QJsonObject &obj)
{
    nlohmann::json result = nlohmann::json::object();

    for (const auto &[key, value] : obj.asKeyValueRange())
        result[key.toString().toUtf8().toStdString()] = toInjaJson(value);

    return result;
}

/*!
    \brief Converts a QJsonArray, \a array, to nlohmann::json.

    Recursively converts all elements in the array, preserving order.
    Mixed-type arrays are supported.

    Returns the equivalent nlohmann::json array.
*/
nlohmann::json InjaBridge::toInjaJson(const QJsonArray &array)
{
    nlohmann::json result = nlohmann::json::array();

    for (const QJsonValue &value : array)
        result.push_back(toInjaJson(value));

    return result;
}

/*!
    \brief Renders a template string, \a templateStr, with provided \a data.

    Uses Inja to render the template with the given JSON data. The data
    is automatically converted from QJsonObject to nlohmann::json.

    The Inja template string, \a templateStr, supports Jinja2 syntax. \a data is
    the JSON data to use for rendering.

    Returns the rendered template as a QString.
*/
QString InjaBridge::render(const QString &templateStr, const QJsonObject &data)
{
    inja::Environment env;
    nlohmann::json jsonData = toInjaJson(data);

    std::string templateUtf8 = templateStr.toUtf8().toStdString();
    std::string resultUtf8 = env.render(templateUtf8, jsonData);

    return QString::fromUtf8(resultUtf8.c_str());
}

/*!
    \brief Renders a template file with provided data.

    Loads and renders a template from the filesystem, using \a templatePath
    which holds the absolute path to the template file. The file should use
    Inja/Jinja2 syntax.  \a data is the JSON data to use for rendering.

    Returns the rendered template as a QString.
*/
QString InjaBridge::renderFile(const QString &templatePath, const QJsonObject &data)
{
    inja::Environment env;
    nlohmann::json jsonData = toInjaJson(data);

    std::string pathUtf8 = templatePath.toUtf8().toStdString();
    std::string resultUtf8 = env.render_file(pathUtf8, jsonData);

    return QString::fromUtf8(resultUtf8.c_str());
}

QT_END_NAMESPACE

