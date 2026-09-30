// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INJABRIDGE_H
#define INJABRIDGE_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_EXCEPTIONS) && !defined(INJA_NOEXCEPTION) && !defined(JSON_NOEXCEPTION) \
        && (__EXCEPTIONS || _CPPUNWIND)
#  define QDOC_TEMPLATE_LIBS_THROW 1
#else
#  define QDOC_TEMPLATE_LIBS_THROW 0
#endif

#if !QDOC_TEMPLATE_LIBS_THROW
// Preserve the failing render's context when library errors cannot
// reach a catch boundary.
QT_BEGIN_NAMESPACE
[[noreturn]] void qdocFatalTemplateRenderError(const char *what);
QT_END_NAMESPACE

#  define INJA_THROW(exception) \
      QT_PREPEND_NAMESPACE(qdocFatalTemplateRenderError)((exception).what())
#  define JSON_THROW_USER(exception) \
      QT_PREPEND_NAMESPACE(qdocFatalTemplateRenderError)((exception).what())
#endif

// nlohmann::json comes in through inja; both redefinitions above must
// precede this include.
#include <inja/inja.hpp>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QString>

#include <functional>
#include <optional>
#include <variant>

QT_BEGIN_NAMESPACE

class InjaBridge
{
public:
    using IncludeCallback = std::function<std::optional<QString>(const QString &name)>;

    struct RenderContext
    {
        QString format;
        QString page;
        QString templatePath;
    };

    struct RenderFailure
    {
        QString message;
    };

    using RenderResult = std::variant<QString, RenderFailure>;

    static QString renderErrorText(const RenderContext &context, const QString &message);

    static nlohmann::json toInjaJson(const QJsonValue &value);
    static nlohmann::json toInjaJson(const QJsonObject &obj);
    static nlohmann::json toInjaJson(const QJsonArray &array);

    static RenderResult render(const QString &templateStr, const QJsonObject &data,
                               const RenderContext &context = { });
    static RenderResult render(const QString &templateStr, const QJsonObject &data,
                               const IncludeCallback &includeCallback,
                               const RenderContext &context = { });
    static RenderResult renderFile(const QString &templatePath, const QJsonObject &data,
                                   const RenderContext &context = { });

private:
    InjaBridge() = default;
};

QT_END_NAMESPACE

#endif // INJABRIDGE_H
