// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_CLASSIFICATIONJSON_H
#define QDOC_IR_CLASSIFICATIONJSON_H

#include "qdoc/access.h"
#include "qdoc/genustypes.h"
#include "qdoc/status.h"

#include <QJsonObject>
#include <QString>

#include <optional>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

// Returns a JSON object with "id" and "label" keys.
// Classification fields use a stable kebab-case id for template
// conditionals and a human-readable label for display.
inline QJsonObject classificationObject(const QString &id, const QString &label)
{
    QJsonObject obj;
    obj["id"_L1] = id;
    obj["label"_L1] = label;
    return obj;
}

// Returns {id, label} for a NodeType, or std::nullopt for NoType.
inline std::optional<QJsonObject> nodeTypeToJson(NodeType t)
{
    switch (t) {
    case NodeType::NoType:
        return std::nullopt;
    case NodeType::Namespace:
        return classificationObject("namespace"_L1, "Namespace"_L1);
    case NodeType::Class:
        return classificationObject("class"_L1, "Class"_L1);
    case NodeType::Struct:
        return classificationObject("struct"_L1, "Struct"_L1);
    case NodeType::Union:
        return classificationObject("union"_L1, "Union"_L1);
    case NodeType::HeaderFile:
        return classificationObject("header-file"_L1, "Header file"_L1);
    case NodeType::Page:
        return classificationObject("page"_L1, "Page"_L1);
    case NodeType::Enum:
        return classificationObject("enum"_L1, "Enum"_L1);
    case NodeType::Example:
        return classificationObject("example"_L1, "Example"_L1);
    case NodeType::ExternalPage:
        return classificationObject("external-page"_L1, "External page"_L1);
    case NodeType::TypeAlias:
        return classificationObject("type-alias"_L1, "Type alias"_L1);
    case NodeType::Typedef:
        return classificationObject("typedef"_L1, "Typedef"_L1);
    case NodeType::Function:
        return classificationObject("function"_L1, "Function"_L1);
    case NodeType::Property:
        return classificationObject("property"_L1, "Property"_L1);
    case NodeType::Proxy:
        return classificationObject("proxy"_L1, "Proxy"_L1);
    case NodeType::Variable:
        return classificationObject("variable"_L1, "Variable"_L1);
    case NodeType::Group:
        return classificationObject("group"_L1, "Group"_L1);
    case NodeType::Module:
        return classificationObject("module"_L1, "Module"_L1);
    case NodeType::QmlType:
        return classificationObject("qml-type"_L1, "QML type"_L1);
    case NodeType::QmlValueType:
        return classificationObject("qml-value-type"_L1, "QML value type"_L1);
    case NodeType::QmlModule:
        return classificationObject("qml-module"_L1, "QML module"_L1);
    case NodeType::QmlProperty:
        return classificationObject("qml-property"_L1, "QML property"_L1);
    case NodeType::QmlEnum:
        return classificationObject("qml-enum"_L1, "QML enum"_L1);
    case NodeType::SharedComment:
        return classificationObject("shared-comment"_L1, "Shared comment"_L1);
    case NodeType::Collection:
        return classificationObject("collection"_L1, "Collection"_L1);
    }
    Q_UNREACHABLE();
}

// Returns {id, label} for a Genus, or std::nullopt for DontCare.
inline std::optional<QJsonObject> genusToJson(Genus g)
{
    switch (g) {
    case Genus::DontCare:
        return std::nullopt;
    case Genus::CPP:
        return classificationObject("cpp"_L1, "C++"_L1);
    case Genus::QML:
        return classificationObject("qml"_L1, "QML"_L1);
    case Genus::DOC:
        return classificationObject("doc"_L1, "Documentation"_L1);
    case Genus::API:
        return classificationObject("api"_L1, "API"_L1);
    }
    Q_UNREACHABLE();
}

// Returns {id, label} for a Status value.
inline QJsonObject statusToJson(Status s)
{
    switch (s) {
    case Status::Deprecated:
        return classificationObject("deprecated"_L1, "Deprecated"_L1);
    case Status::Preliminary:
        return classificationObject("preliminary"_L1, "Preliminary"_L1);
    case Status::Active:
        return classificationObject("active"_L1, "Active"_L1);
    case Status::Internal:
        return classificationObject("internal"_L1, "Internal"_L1);
    case Status::DontDocument:
        return classificationObject("ignored"_L1, "Ignored"_L1);
    }
    Q_UNREACHABLE();
}

// Returns {id, label} for an Access value.
inline QJsonObject accessToJson(Access a)
{
    switch (a) {
    case Access::Public:
        return classificationObject("public"_L1, "Public"_L1);
    case Access::Protected:
        return classificationObject("protected"_L1, "Protected"_L1);
    case Access::Private:
        return classificationObject("private"_L1, "Private"_L1);
    }
    Q_UNREACHABLE();
}

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_CLASSIFICATIONJSON_H
