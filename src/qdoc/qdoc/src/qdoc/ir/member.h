// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_MEMBER_H
#define QDOC_IR_MEMBER_H

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

    [[nodiscard]] QJsonObject toJson() const;
};

struct InheritedMembersIR {
    QString className;
    int count { 0 };
    QString href;
    [[nodiscard]] QJsonObject toJson() const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_MEMBER_H
