// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INJABRIDGE_H
#define INJABRIDGE_H

#include <inja/inja.hpp>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>

QT_BEGIN_NAMESPACE

class InjaBridge
{
public:
    static nlohmann::json toInjaJson(const QJsonValue &value);
    static nlohmann::json toInjaJson(const QJsonObject &obj);
    static nlohmann::json toInjaJson(const QJsonArray &array);

    static QString render(const QString &templateStr, const QJsonObject &data);
    static QString renderFile(const QString &templatePath, const QJsonObject &data);

private:
    InjaBridge() = default;
};

QT_END_NAMESPACE

#endif // INJABRIDGE_H


