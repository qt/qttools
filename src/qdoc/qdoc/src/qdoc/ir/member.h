// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_MEMBER_H
#define QDOC_IR_MEMBER_H

#include <QJsonObject>
#include <QString>

QT_BEGIN_NAMESPACE

namespace IR {

struct ParameterIR {
    QString type;
    QString name;
    QString defaultValue;
    [[nodiscard]] QJsonObject toJson() const;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_MEMBER_H
