// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "injabridge.h"

#include "textutils.h"

#include <cmath>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

static std::string escapeHtml(const std::string &input)
{
    std::string buffer;
    buffer.reserve(input.size() + input.size() / 8);
    for (char c : input) {
        switch (c) {
        case '&':
            buffer += "&amp;";
            break;
        case '"':
            buffer += "&quot;";
            break;
        case '\'':
            buffer += "&apos;";
            break;
        case '<':
            buffer += "&lt;";
            break;
        case '>':
            buffer += "&gt;";
            break;
        default:
            buffer += c;
            break;
        }
    }
    return buffer;
}

// Render a single link span for either output surface. HTML escapes both the
// visible text and the href; a span with no usable href degrades to plain text
// rather than a dangling link. Markdown emits a bracket-paren link with the
// text and URL as-is: the concept names and generated hrefs in scope here carry
// no Markdown-special characters, and general Markdown escaping is out of scope.
static std::string renderLinkSpan(const std::string &text, const std::string &href,
                                  bool hasHref, bool isMarkdown)
{
    if (isMarkdown)
        return hasHref ? "[" + text + "](" + href + ")" : text;
    if (hasHref)
        return "<a href=\"" + escapeHtml(href) + "\">" + escapeHtml(text) + "</a>";
    return escapeHtml(text);
}

// Render a span subtree as native Markdown. Any span carrying an href — a
// concept link, a linked type, an external reference — becomes a Markdown
// link; every other role contributes its text and recurses into its children,
// so structural roles (type, name, template-decl, the [signal]/[slot] tags)
// stay plain text instead of HTML markup. Markdown-special characters in the
// signature text are not escaped here; see the Markdown-first-class follow-up.
static std::string renderSpanMarkdown(const nlohmann::json &s)
{
    const auto text = s.value("text", "");
    const auto href = s.value("href", "");
    const bool hasHref = s.contains("href") && !href.empty();
    if (hasHref)
        return renderLinkSpan(text, href, hasHref, /*isMarkdown=*/true);

    std::string result = text;
    if (s.contains("children") && s["children"].is_array()) {
        for (const auto &c : s["children"])
            result += renderSpanMarkdown(c);
    }
    return result;
}

static std::string renderSignatureSpans(const nlohmann::json &spans, const QString &format)
{
    if (!spans.is_array()) {
        qWarning("render_signature_spans: expected JSON array, got %s",
                 spans.type_name());
        return {};
    }

    const bool isMarkdown = format.contains("markdown"_L1, Qt::CaseInsensitive);

    std::string result;
    for (const auto &s : spans) {
        if (isMarkdown) {
            result += renderSpanMarkdown(s);
            continue;
        }
        const auto role = s.value("role", "");
        const auto text = s.value("text", "");
        const bool hasHref = s.contains("href");
        const auto href = s.value("href", "");

        if (role == "extra") {
            result += R"(<code class="details extra" translate="no">)";
            if (s.contains("children") && s["children"].is_array()) {
                for (const auto &c : s["children"]) {
                    if (c.value("role", "") == "external-ref")
                        result += "<a href=\"" + c.value("href", "") + "\">"
                                + escapeHtml(c.value("text", "")) + "</a>";
                    else
                        result += escapeHtml(c.value("text", ""));
                }
            } else {
                result += escapeHtml(text);
            }
            result += "</code>";
        } else if (role == "type") {
            result += R"(<span class="type">)";
            if (hasHref)
                result += "<a href=\"" + href + "\">";
            result += escapeHtml(text);
            if (hasHref)
                result += "</a>";
            result += "</span>";
        } else if (role == "name") {
            result += R"(<span class="name">)";
            if (hasHref)
                result += "<a href=\"" + href + "\">";
            result += escapeHtml(text);
            if (hasHref)
                result += "</a>";
            result += "</span>";
        } else if (role == "parameter") {
            result += "<i>" + escapeHtml(text) + "</i>";
        } else if (role == "external-ref") {
            result += "<a href=\"" + href + "\">" + escapeHtml(text) + "</a>";
        } else if (role == "template-decl") {
            result += R"(<span class="template-decl">)";
            result += escapeHtml(text);
            if (s.contains("children") && s["children"].is_array()) {
                for (const auto &c : s["children"]) {
                    const auto childRole = c.value("role", "");
                    const auto childText = c.value("text", "");
                    const bool childHasHref = c.contains("href");
                    const auto childHref = c.value("href", "");
                    if (childRole == "type") {
                        result += R"(<span class="type">)" + escapeHtml(childText)
                                + "</span>";
                    } else if (childRole == "link") {
                        result += renderLinkSpan(childText, childHref, childHasHref, isMarkdown);
                    } else {
                        result += escapeHtml(childText);
                    }
                }
            }
            result += "</span>";
        } else if (role == "link") {
            result += renderLinkSpan(text, href, hasHref, isMarkdown);
        } else {
            result += escapeHtml(text);
        }
    }
    return result;
}

static void registerCallbacks(inja::Environment &env, const QString &format)
{
    env.add_callback("escape_html", 1, [](inja::Arguments &args) {
        return escapeHtml(args.at(0)->get<std::string>());
    });

    env.add_callback("render_signature_spans", 1, [format](inja::Arguments &args) {
        return renderSignatureSpans(*args.at(0), format);
    });

    // English-list punctuation for templates that iterate over a list of
    // items. Emit {{ list_separator(loop.index, length(items)) }} after
    // each item instead of a literal comma or period so the rendered
    // output matches the legacy HTML generator's prose (for instance
    // "See also a(), b(), and c." rather than a comma-less concatenation).
    // loop.index is zero-based in Inja; TextUtils::separator expects the
    // same convention.
    env.add_callback("list_separator", 2, [](inja::Arguments &args) {
        const auto pos = args.at(0)->get<qsizetype>();
        const auto total = args.at(1)->get<qsizetype>();
        return TextUtils::separator(pos, total).toStdString();
    });

    env.add_callback("escape_md_table", 1, [](inja::Arguments &args) {
        auto input = args.at(0)->get<std::string>();
        std::string buffer;
        buffer.reserve(input.size() + input.size() / 8);
        for (char c : input) {
            switch (c) {
            case '|':
                buffer += "\\|";
                break;
            case '\n':
                buffer += ' ';
                break;
            case '\r':
                break;
            default:
                buffer += c;
                break;
            }
        }
        return buffer;
    });

    // Parity helper for alternating-row table styling. Templates use
    // {{ is_odd(loop.index1) }} to choose between "odd" and "even"
    // class names, matching the legacy HtmlGenerator's tr.odd / tr.even
    // contract. Implemented as a callback rather than relying on Inja's
    // expression grammar so the parity check is unambiguous regardless
    // of which arithmetic operators Inja supports in any given release.
    env.add_callback("is_odd", 1, [](inja::Arguments &args) {
        return args.at(0)->get<qsizetype>() % 2 != 0;
    });
}

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

    \note All numbers in QJsonValue are stored as doubles. Whole-number doubles
    (e.g., 30.0, 2.0) are converted to int64_t so that template output renders
    them as integers (e.g., "30" not "30.0"). Fractional values pass through
    as doubles.

    \note Inja and nlohmann::json may report template or data errors. QDoc is
    built with exceptions disabled (\c{-fno-exceptions}), so such errors are
    treated as fatal and will terminate the process. A custom \c INJA_THROW
    override in the header ensures that error details (including source
    location) are logged via \c qFatal() before termination, rather than
    calling \c std::abort() silently.

    All render methods register template callbacks:
    \list
    \li \c{escape_html()} escapes HTML special characters (\c{&}, \c{<},
        \c{>}, \c{"}, \c{'}).
    \li \c{escape_md_table()} escapes pipe characters and collapses newlines
        for safe use inside Markdown table cells.
    \li \c{render_signature_spans()} converts a JSON array of signature spans
        (from SignatureSpan IR) into semantic HTML with role-based markup
        (\c{<span class="type">}, \c{<span class="name">}, etc.).
    \endlist
    Escaping callbacks keep format-specific logic under template author control.
    The \c{render_signature_spans()} callback centralizes signature rendering
    that was previously duplicated across multiple templates.

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
    case QJsonValue::Double: {
        double d = value.toDouble();
        if (std::fmod(d, 1.0) == 0.0)
            return static_cast<int64_t>(d);
        return d;
    }
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
QString InjaBridge::render(const QString &templateStr, const QJsonObject &data,
                           const QString &format)
{
    inja::Environment env;
    // Replace Inja's default "##" line statement prefix, which conflicts
    // with Markdown headings. "%!" echoes Jinja2's "%" (statement) and
    // QDoc's "!" (documentation marker), and is inert in both HTML and
    // Markdown.
    env.set_line_statement("%!");
    env.set_trim_blocks(true);
    env.set_lstrip_blocks(true);
    registerCallbacks(env, format);
    nlohmann::json jsonData = toInjaJson(data);

    std::string templateUtf8 = templateStr.toUtf8().toStdString();
    std::string resultUtf8 = env.render(templateUtf8, jsonData);

    return QString::fromUtf8(resultUtf8.c_str());
}

/*!
    \brief Renders a template string, \a templateStr, with provided \a data,
    using \a includeCallback to resolve \c{{% include %}} directives.

    This overload configures the Inja environment with a custom include
    callback so that templates can use \c{{% include "name" %}} directives.
    The \a includeCallback receives the include name and returns the partial's
    content as a QString. If the callback returns an empty string, the include
    is treated as missing and a fatal error is raised.

    This enables Inja's include mechanism to work with Qt's resource system,
    where \c{std::ifstream} cannot open \c{:/} paths.

    Returns the rendered template as a QString.
*/
QString InjaBridge::render(const QString &templateStr, const QJsonObject &data,
                           const IncludeCallback &includeCallback,
                           const QString &format)
{
    inja::Environment env;
    env.set_line_statement("%!");
    env.set_trim_blocks(true);
    env.set_lstrip_blocks(true);
    registerCallbacks(env, format);
    env.set_search_included_templates_in_files(false);
    env.set_include_callback(
            [&includeCallback, &env](const std::filesystem::path & /*path*/,
                                     const std::string &name) -> inja::Template {
                QString content = includeCallback(QString::fromStdString(name));
                if (content.isEmpty()) {
                    INJA_THROW(
                            inja::FileError("include not found: '" + name + "'"));
                }
                return env.parse(content.toUtf8().toStdString());
            });

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
QString InjaBridge::renderFile(const QString &templatePath, const QJsonObject &data,
                               const QString &format)
{
    inja::Environment env;
    env.set_line_statement("%!");
    env.set_trim_blocks(true);
    env.set_lstrip_blocks(true);
    registerCallbacks(env, format);
    nlohmann::json jsonData = toInjaJson(data);

    std::string pathUtf8 = templatePath.toUtf8().toStdString();
    std::string resultUtf8 = env.render_file(pathUtf8, jsonData);

    return QString::fromUtf8(resultUtf8.c_str());
}

QT_END_NAMESPACE

