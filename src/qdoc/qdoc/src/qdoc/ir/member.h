// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_MEMBER_H
#define QDOC_IR_MEMBER_H

#include "contentblock.h"
#include "qdoc/access.h"
#include "qdoc/genustypes.h"
#include "qdoc/status.h"

#include <QJsonObject>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE

namespace IR {

struct ParameterIR {
    QString type;
    QString name;
    QString defaultValue;
    [[nodiscard]] QJsonObject toJson() const;
};

struct EnumValueIR {
    QString name;
    QString value;
    QString since;
    [[nodiscard]] QJsonObject toJson() const;
};

struct MemberIR {
    QString name;
    QString fullName;
    QString signature;
    QString href;
    QString brief;

    NodeType nodeType { NodeType::NoType };
    Access access { Access::Public };
    Status status { Status::Active };

    QList<ParameterIR> parameters;
    int overloadNumber { 0 };
    bool isPrimaryOverload { true };

    QList<EnumValueIR> enumValues;

    bool isStatic { false };
    bool isConst { false };
    bool isVirtual { false };
    bool isSignal { false };
    bool isSlot { false };

    // QML property attributes
    bool isAttached { false };
    bool isDefault { false };
    bool isReadOnly { false };
    bool isRequired { false };
    QString dataType;

    // Detail documentation (populated for detail sections, empty for summary)
    QString anchorId;
    QString synopsis;
    QString since;
    QString threadSafety;
    QString comparisonCategory;
    bool isNoexcept { false };
    QString noexceptNote;
    QList<ContentBlock> body;
    QList<ContentBlock> alsoList;

    [[nodiscard]] QJsonObject toJson() const;
};

struct InheritedMembersIR {
    QString className;
    int count { 0 };
    QString href;
    [[nodiscard]] QJsonObject toJson() const;
};

struct SectionIR {
    QString id;
    QString title;
    QString singular;
    QString plural;
    QList<MemberIR> members;
    QList<MemberIR> reimplementedMembers;
    QList<InheritedMembersIR> inheritedMembers;
    [[nodiscard]] QJsonObject toJson() const;
};

struct AllMemberEntry {
    QString signature;
    QString href;
    QStringList hints;
    bool isPropertyGroup { false };
    QList<AllMemberEntry> children;
    [[nodiscard]] QJsonObject toJson() const;
};

struct MemberGroup {
    QString typeName;
    QString typeHref;
    QList<AllMemberEntry> members;
    [[nodiscard]] QJsonObject toJson() const;
};

struct AllMembersIR {
    QString typeName;
    QString typeHref;
    bool isQmlType { false };
    QList<AllMemberEntry> members;
    QList<MemberGroup> memberGroups;
    [[nodiscard]] QJsonObject toJson() const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_MEMBER_H
