// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "member.h"

#include "classificationjson.h"

#include <QJsonArray>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \struct IR::ParameterIR
    \brief Intermediate representation of a function parameter.

    ParameterIR captures the type, name, and optional default value
    of a single function parameter. Templates use this to render
    parameter lists in function synopses.

    JSON output omits \c defaultValue when the string is empty,
    following the convention of suppressing absent optional fields.
*/

/*!
    \variable IR::ParameterIR::type
    Parameter type (such as "const QString &").
*/

/*!
    \variable IR::ParameterIR::name
    Parameter name.
*/

/*!
    \variable IR::ParameterIR::defaultValue
    Default value expression, empty if none.
*/

/*!
    Converts the parameter to a QJsonObject for template rendering.

    Always emits \c type and \c name. The \c defaultValue key is
    omitted when the default value string is empty.
*/
QJsonObject ParameterIR::toJson() const
{
    QJsonObject json;
    json["type"_L1] = type;
    json["name"_L1] = name;
    if (!defaultValue.isEmpty())
        json["defaultValue"_L1] = defaultValue;
    return json;
}

/*!
    \struct IR::EnumValueIR
    \brief Intermediate representation of a single enum value.

    EnumValueIR captures the name, explicit initializer, and version
    information for one enumerator. Templates use this to render
    enum value tables in class documentation.

    JSON output omits \c value and \c since when their respective
    strings are empty.
*/

/*!
    \variable IR::EnumValueIR::name
    Enumerator name.
*/

/*!
    \variable IR::EnumValueIR::value
    Explicit initializer expression, empty if the compiler assigns
    the value.
*/

/*!
    \variable IR::EnumValueIR::since
    Qt version that introduced this enumerator, empty if unversioned.
*/

/*!
    Converts the enum value to a QJsonObject for template rendering.

    Always emits \c name. The \c value and \c since keys are omitted
    when their respective strings are empty.
*/
QJsonObject EnumValueIR::toJson() const
{
    QJsonObject json;
    json["name"_L1] = name;
    if (!value.isEmpty())
        json["value"_L1] = value;
    if (!since.isEmpty())
        json["since"_L1] = since;
    return json;
}

/*!
    \struct IR::MemberIR
    \brief Intermediate representation of a single documentable member.

    MemberIR captures identity, classification, and type-specific
    metadata for one member of an aggregate (such as a class,
    namespace, or QML type). Function members carry parameter lists
    and overload metadata; enum members carry value listings.
    Templates use this to render summary tables and detail sections.

    JSON output omits \c parameters and \c enumValues when the
    respective lists are empty. The \c nodeType field is omitted
    when set to NoType.
*/

/*!
    \variable IR::MemberIR::name
    Unqualified member name.
*/

/*!
    \variable IR::MemberIR::fullName
    Fully qualified name including the enclosing scope.
*/

/*!
    \variable IR::MemberIR::signature
    Display signature for synopsis rendering. The format depends on
    the member type: functions include return type and default values,
    properties use "name : type", and enums include the scoped or
    unscoped distinction.
*/

/*!
    \variable IR::MemberIR::href
    URL of the member's detailed documentation.
*/

/*!
    \variable IR::MemberIR::brief
    One-line summary extracted from the member's doc comment,
    empty if none.
*/

/*!
    \variable IR::MemberIR::nodeType
    Classification of the member's entity type (function, property,
    enum, and so on). Defaults to NoType.
*/

/*!
    \variable IR::MemberIR::access
    Access level (public, protected, or private). Defaults to Public.
*/

/*!
    \variable IR::MemberIR::status
    Documentation status (active, deprecated, preliminary, or
    internal). Defaults to Active.
*/

/*!
    \variable IR::MemberIR::parameters
    Parameter list for function members. Empty for non-functions.
*/

/*!
    \variable IR::MemberIR::overloadNumber
    Zero-based overload index. Zero indicates the primary overload.
*/

/*!
    \variable IR::MemberIR::isPrimaryOverload
    Whether this is the primary (first) overload of its name.
    Defaults to true.
*/

/*!
    \variable IR::MemberIR::enumValues
    Value list for enum members. Empty for non-enums.
*/

/*!
    \variable IR::MemberIR::isStatic
    Whether the member is declared static.
*/

/*!
    \variable IR::MemberIR::isConst
    Whether the member is declared const.
*/

/*!
    \variable IR::MemberIR::isVirtual
    Whether the member is virtual (including pure virtual and
    override).
*/

/*!
    \variable IR::MemberIR::isSignal
    Whether the member is a Qt signal.
*/

/*!
    \variable IR::MemberIR::isSlot
    Whether the member is a Qt slot.
*/

/*!
    Converts the member to a QJsonObject for template rendering.

    Emits identity fields (name, fullName, signature, href),
    classification as \c {id, label} objects, overload metadata, and
    qualifier flags. The \c brief, \c parameters, and \c enumValues
    fields are omitted when empty. The \c nodeType field is omitted
    when NoType.
*/
QJsonObject MemberIR::toJson() const
{
    QJsonObject json;

    json["name"_L1] = name;
    json["fullName"_L1] = fullName;
    json["signature"_L1] = signature;
    json["href"_L1] = href;
    if (!brief.isEmpty())
        json["brief"_L1] = brief;

    if (const auto t = nodeTypeToJson(nodeType))
        json["nodeType"_L1] = *t;
    json["status"_L1] = statusToJson(status);
    json["access"_L1] = accessToJson(access);

    json["overloadNumber"_L1] = overloadNumber;
    json["isPrimaryOverload"_L1] = isPrimaryOverload;

    if (!parameters.isEmpty()) {
        QJsonArray arr;
        for (const auto &p : parameters)
            arr.append(p.toJson());
        json["parameters"_L1] = arr;
    }

    if (!enumValues.isEmpty()) {
        QJsonArray arr;
        for (const auto &ev : enumValues)
            arr.append(ev.toJson());
        json["enumValues"_L1] = arr;
    }

    json["isStatic"_L1] = isStatic;
    json["isConst"_L1] = isConst;
    json["isVirtual"_L1] = isVirtual;
    json["isSignal"_L1] = isSignal;
    json["isSlot"_L1] = isSlot;

    return json;
}

/*!
    \struct IR::InheritedMembersIR
    \brief Summary of members inherited from a single base class.

    InheritedMembersIR stores a count and link target for one base
    class's contributed members. Templates use this to render lines
    such as "5 public functions inherited from QObject" at the end
    of a section.
*/

/*!
    \variable IR::InheritedMembersIR::className
    Fully qualified name of the base class.
*/

/*!
    \variable IR::InheritedMembersIR::count
    Number of members inherited from this base class.
*/

/*!
    \variable IR::InheritedMembersIR::href
    URL of the base class's documentation page.
*/

/*!
    Converts the inherited members summary to a QJsonObject.

    Emits \c className, \c count, and \c href. Templates combine
    this with the enclosing section's \c plural field to render
    links such as "5 public functions inherited from QObject".
*/
QJsonObject InheritedMembersIR::toJson() const
{
    QJsonObject json;
    json["className"_L1] = className;
    json["count"_L1] = count;
    json["href"_L1] = href;
    return json;
}

/*!
    \struct IR::SectionIR
    \brief Intermediate representation of a member summary section.

    SectionIR groups members by category (such as "Public Functions"
    or "Properties") for summary table rendering. Each section carries
    a title, singular and plural forms for inherited-member links, and
    three member lists: primary members, reimplemented members, and
    inherited member summaries.

    The \c reimplementedMembers and \c inheritedMembers arrays are
    omitted from JSON when empty.
*/

/*!
    \variable IR::SectionIR::id
    Stable ASCII identifier for anchor links, generated from the
    title via Utilities::asAsciiPrintable().
*/

/*!
    \variable IR::SectionIR::title
    Display title (such as "Public Functions" or "Properties").
*/

/*!
    \variable IR::SectionIR::singular
    Singular form of the member type (such as "function").
*/

/*!
    \variable IR::SectionIR::plural
    Plural form of the member type (such as "functions").
*/

/*!
    \variable IR::SectionIR::members
    Primary members in this section.
*/

/*!
    \variable IR::SectionIR::reimplementedMembers
    Members that reimplement a virtual function from a base class.
*/

/*!
    \variable IR::SectionIR::inheritedMembers
    Summaries of members inherited from base classes.
*/

/*!
    Converts the section to a QJsonObject for template rendering.

    Emits \c id, \c title, \c singular, \c plural, and the \c members
    array. The \c reimplementedMembers and \c inheritedMembers arrays
    are omitted when empty.
*/
QJsonObject SectionIR::toJson() const
{
    QJsonObject json;

    json["id"_L1] = id;
    json["title"_L1] = title;
    json["singular"_L1] = singular;
    json["plural"_L1] = plural;

    QJsonArray membersArray;
    for (const auto &m : members)
        membersArray.append(m.toJson());
    json["members"_L1] = membersArray;

    if (!reimplementedMembers.isEmpty()) {
        QJsonArray arr;
        for (const auto &m : reimplementedMembers)
            arr.append(m.toJson());
        json["reimplementedMembers"_L1] = arr;
    }

    if (!inheritedMembers.isEmpty()) {
        QJsonArray arr;
        for (const auto &im : inheritedMembers)
            arr.append(im.toJson());
        json["inheritedMembers"_L1] = arr;
    }

    return json;
}

} // namespace IR

QT_END_NAMESPACE
