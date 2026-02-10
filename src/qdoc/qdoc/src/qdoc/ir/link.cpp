// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "link.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \struct IR::Link
    \brief Intermediate representation for a resolved hyperlink.

    Link represents a fully-resolved link in QDoc's intermediate representation
    layer. All target resolution happens during the IR building phase, so templates
    receive only pre-computed URLs and display text. This eliminates the need for
    generators to perform database lookups during output generation.

    \sa IR::Document
*/

/*!
    \enum IR::Link::State
    Indicates the resolution state of the link target.

    \value Resolved Link target was found and resolved.
    \value External Link points to an external resource.
    \value Unresolved Target not found (will produce warning).
    \value Broken Target explicitly marked as broken.
*/


/*!
    Converts the Link to a QJsonObject for template rendering.

    The JSON structure uses camelCase field names following the established
    IR-to-JSON convention. The state is represented as a string for readability
    in templates: "resolved", "external", "unresolved", or "broken".

    Returns a QJsonObject containing all link data suitable for template
    rendering via InjaBridge.
*/
QJsonObject Link::toJson() const
{
    QJsonObject json;
    json["target"_L1] = target;
    json["text"_L1] = text;

    if (!title.isEmpty())
        json["title"_L1] = title;

    QString stateStr;
    switch (state) {
    case State::Resolved:
        stateStr = "resolved"_L1;
        break;
    case State::External:
        stateStr = "external"_L1;
        break;
    case State::Unresolved:
        stateStr = "unresolved"_L1;
        break;
    case State::Broken:
        stateStr = "broken"_L1;
        break;
    }
    json["state"_L1] = stateStr;

    json["isResolved"_L1] = (state == State::Resolved);
    json["isExternal"_L1] = (state == State::External);

    if (!originalTarget.isEmpty() && originalTarget != target)
        json["originalTarget"_L1] = originalTarget;

    return json;
}

} // namespace IR

QT_END_NAMESPACE

