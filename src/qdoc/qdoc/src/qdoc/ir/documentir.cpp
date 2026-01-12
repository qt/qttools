// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "documentir.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

/*!
    \struct DocumentIR
    \brief Intermediate representation for a documentation topic.

    DocumentIR contains all information needed to render a single documentation
    page using templates. All links are pre-resolved, sections are pre-organized,
    and file paths are pre-computed. The template engine receives only this IR
    and performs no lookups or resolution itself.
*/

/*!
    Converts the DocumentIR to a QJsonObject for template rendering.

    The JSON structure follows a convention where field names use camelCase
    and match template variable names. This IR-to-JSON conversion is intentionally
    simple for now, containing only the fields needed for basic page rendering.

    The \c contentJson field is nested under a 'content' key to provide better
    structure and namespace separation in templates.

    Returns a QJsonObject containing all IR data in a format suitable for
    passing to the Inja template engine via InjaBridge.

*/
QJsonObject DocumentIR::toJson() const
{
    QJsonObject json;
    json["title"_L1] = title;
    json["fullTitle"_L1] = fullTitle;
    json["url"_L1] = url;
    json["brief"_L1] = brief;

    if (!contentJson.isEmpty())
        json["content"_L1] = contentJson;

    return json;
}

QT_END_NAMESPACE

