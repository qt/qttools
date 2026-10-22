// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "outputconfiguration.h"

#include "config.h"

#include <QtCore/qregularexpression.h>

#include <optional>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \enum Pipeline
    \internal
    \inmodule QDoc
    \relates OutputSelection
    \brief Identifies an implementation that can own publication formats.

    Format identity is independent of the implementation that renders it.
    Selecting a pipeline does not require it to own any document format.

    \value Legacy The compiled-in legacy generators.
    \value Template The template engine with its effective package capabilities.
*/

/*!
    \struct PipelineCapabilities
    \internal
    \inmodule QDoc
    \brief Describes a pipeline without constructing generators or loading packages.

    The caller supplies compiled-in legacy capabilities or the effective
    template package's capabilities. Resolution reads these values without
    retaining references to them. An unavailable selected pipeline is an error.

    A declared but broken format remains an ownership claim even when it is
    absent from \c{supportedFormats}. A broken template declaration therefore
    prevents fallback to legacy. Extensions supply defaults only; they do not
    determine format identity, ownership, or template selection.
*/

/*!
    \variable PipelineCapabilities::pipeline
    \brief The pipeline whose capabilities this record describes.
*/

/*!
    \variable PipelineCapabilities::available
    \brief Whether this pipeline can run in the invoking build and environment.

    Set this to \c false for a compiled-out engine or an unusable package.
*/

/*!
    \variable PipelineCapabilities::supportedFormats
    \brief The case-sensitive format identities this pipeline can render.
*/

/*!
    \variable PipelineCapabilities::brokenFormats
    \brief The declared formats whose requirements the pipeline cannot satisfy.

    These identities need not also appear in \c{supportedFormats}. A requested
    broken declaration is an error, not permission to choose another owner.
*/

/*!
    \variable PipelineCapabilities::formatExtensions
    \brief The pipeline or effective-package extension defaults, keyed by format.

    Resolution applies an explicit \c{<Format>.extension} override. Each
    requested format needs a non-empty effective extension; unfamiliar formats
    do not inherit another format's extension.
*/

/*!
    \struct OutputConfigurationInput
    \internal
    \inmodule QDoc
    \brief Supplies invocation-specific selections and pipeline capabilities.

    The caller supplies command-line values in their original order. A
    non-empty list replaces the corresponding configuration selection; it is
    not additive. Represent an explicit empty argument with an empty string,
    rather than an absent list. These inputs do not expose a public option or
    invoke package loading.
*/

/*!
    \variable OutputConfigurationInput::cliPipelines
    \brief The ordered command-line pipeline selection, or an empty list if absent.
*/

/*!
    \variable OutputConfigurationInput::cliFormats
    \brief The ordered command-line format selection, or an empty list if absent.

    This selection may introduce formats absent from the configuration.
*/

/*!
    \variable OutputConfigurationInput::pipelineCapabilities
    \brief The capability records for pipelines known to the caller.

    Supply one effective record per pipeline. Absence of a selected pipeline's
    record is an error, as is a record whose \c{available} value is false.
*/

/*!
    \struct OutputSelection
    \internal
    \inmodule QDoc
    \brief Resolves format requests and ownership independently of destination layout.

    Explicit pipeline presence in a configuration file or include, or an
    explicit command-line selection, opts in to the new publication model.
    Without either, the result is a compatibility marker; existing runtime
    interpretation retains control.

    The result retains the full configured format order, followed by CLI-only
    identities. Repeated identities keep their first occurrence. Identity
    tokens are trimmed and empty tokens are discarded; path values do not use
    this normalization. Requested flags record CLI replacement without losing
    configured positions needed by subsequent layout planning.

    Template owns its declared formats before legacy, independently of pipeline
    order. Ownership and extension values are usable for requested formats only
    in New mode when hasErrors() returns \c false. Non-requested entries retain
    configured identity and position without requiring a usable owner.

    Resolution never creates output, constructs generators, loads packages, or
    emits diagnostics. The caller handles errors before publication and emits
    migration notices outside the authored-documentation warning budget.
*/

/*!
    \enum OutputSelection::Mode
    \internal
    \brief Identifies which configuration interpretation controls publication.

    \value Compatibility No explicit pipeline selection; use existing runtime behavior.
    \value New Explicit pipeline selection; use the resolved publication model.
*/

/*!
    \struct OutputSelection::Diagnostic
    \internal
    \inmodule QDoc
    \brief Carries a source-positioned publication diagnostic without emitting it.

    Resolution deduplicates equal severity, file, line, message, and details.
    The caller owns presentation and failure handling; this record does not
    increment QDoc's warning or error counters.
*/

/*!
    \enum OutputSelection::Diagnostic::Kind
    \internal
    \brief Distinguishes blocking errors from migration advice.

    \value Error The configuration cannot be published as resolved.
    \value Notice Migration advice, not an authored-documentation warning.
*/

/*!
    \variable OutputSelection::Diagnostic::kind
    \brief The diagnostic's effect on publication validity.
*/

/*!
    \variable OutputSelection::Diagnostic::location
    \brief The responsible configuration declaration, if known.

    CLI-only and programmatic selections can have an empty location.
*/

/*!
    \variable OutputSelection::Diagnostic::message
    \brief The primary explanation of the configuration problem or migration advice.
*/

/*!
    \variable OutputSelection::Diagnostic::details
    \brief Optional context to accompany the primary message.
*/

/*!
    \struct OutputSelection::Format
    \internal
    \inmodule QDoc
    \brief Records a format's identity, request status, owner, and extension.

    A configured but unrequested entry remains in the result to preserve layout
    assignment. Consumers must not mistake it for an additional publication.
*/

/*!
    \variable OutputSelection::Format::format
    \brief The case-sensitive permanent publication identity.
*/

/*!
    \variable OutputSelection::Format::requested
    \brief Whether the effective invocation requests this format.
*/

/*!
    \variable OutputSelection::Format::configured
    \brief Whether the identity occurs in the configured format list, including defaults.
*/

/*!
    \variable OutputSelection::Format::pipeline
    \brief The selected owner, usable only for a valid requested format in New mode.
*/

/*!
    \variable OutputSelection::Format::fileExtension
    \brief The effective extension without an automatically added separator.

    The caller combines this value with the document name. It does not select
    templates or imply a publication destination.
*/

/*!
    \variable OutputSelection::Format::location
    \brief The format-list declaration from which this entry derives.

    This is empty for a CLI-only identity or a programmatic declaration without
    source provenance.
*/

/*!
    \variable OutputSelection::mode
    \brief The configuration interpretation selected for this invocation.
*/

/*!
    \variable OutputSelection::pipelines
    \brief The available enabled pipelines in selection order, without duplicates.

    This order does not determine format ownership. An unavailable selection
    contributes an error rather than an entry in this list.
*/

/*!
    \variable OutputSelection::formats
    \brief The configured formats followed by CLI-only formats, in first-occurrence order.
*/

/*!
    \variable OutputSelection::diagnostics
    \brief The deduplicated errors and notices collected during resolution.
*/

namespace {

using Diagnostic = OutputSelection::Diagnostic;

QString pipelineName(Pipeline pipeline)
{
    return pipeline == Pipeline::Legacy ? u"legacy"_s : u"template"_s;
}

std::optional<Pipeline> pipelineFromName(const QString &name)
{
    if (name == "legacy"_L1)
        return Pipeline::Legacy;
    if (name == "template"_L1)
        return Pipeline::Template;
    return std::nullopt;
}

QString configKey(const QString &format, const QString &suffix)
{
    return format + Config::dot + suffix;
}

bool isValidFormatIdentity(const QString &format)
{
    static const QRegularExpression identity(uR"(^[A-Za-z0-9_]+$)"_s);
    return identity.match(format).hasMatch();
}

QStringList filterIdentityTokens(const QStringList &values)
{
    QStringList result;
    for (const QString &value : values) {
        const QString trimmed = value.trimmed();
        if (!trimmed.isEmpty() && !result.contains(trimmed))
            result.append(trimmed);
    }
    return result;
}

class DiagnosticReporter
{
public:
    explicit DiagnosticReporter(QVector<Diagnostic> &diagnostics) : m_diagnostics(diagnostics)
    {
        for (const auto &diagnostic : diagnostics)
            m_seen.insert(key(diagnostic));
    }

    void add(Diagnostic::Kind kind, const Location &location, QString message,
             QString details = {})
    {
        Diagnostic diagnostic{kind, location, std::move(message), std::move(details)};
        const QString identity = key(diagnostic);
        if (m_seen.contains(identity))
            return;
        m_seen.insert(identity);
        m_diagnostics.push_back(std::move(diagnostic));
    }

private:
    static QString key(const Diagnostic &diagnostic)
    {
        return QString::number(static_cast<int>(diagnostic.kind)) + u'\0'
                + diagnostic.location.filePath() + u'\0'
                + QString::number(diagnostic.location.lineNo()) + u'\0'
                + diagnostic.message + u'\0' + diagnostic.details;
    }

    QVector<Diagnostic> &m_diagnostics;
    QSet<QString> m_seen;
};

} // namespace

/*!
    Resolves publication requests from \a config and the explicit \a input.

    Returns diagnostics as data and leaves both inputs unchanged. Compatibility
    mode returns no format plan but can carry notices for new-only settings
    without explicit pipeline opt-in. In New mode, errors invalidate the result;
    callers must check hasErrors() before using requested owners or extensions.
*/
OutputSelection OutputSelection::resolve(const Config &config,
                                         const OutputConfigurationInput &input)
{
    OutputSelection result;
    DiagnosticReporter reporter(result.diagnostics);
    const bool cliPipelines = !input.cliPipelines.isEmpty();
    if (!cliPipelines && !config.contains(CONFIG_PIPELINE)) {
        const auto flagNewOnlySetting = [&](const QString &var) {
            reporter.add(Diagnostic::Kind::Notice, config.get(var).location(),
                         "New publication setting '%1' has no effect without an explicit "
                         "'pipeline' selection."_L1.arg(var));
        };
        if (config.contains(CONFIG_PUBLICATIONLAYOUT))
            flagNewOnlySetting(CONFIG_PUBLICATIONLAYOUT);
        if (config.contains(CONFIG_TEMPLATEPACKAGE))
            flagNewOnlySetting(CONFIG_TEMPLATEPACKAGE);
        for (const QString &key : config.variableNames()) {
            if (key.endsWith(Config::dot + CONFIG_PUBLICATIONLAYOUT))
                flagNewOnlySetting(key);
        }
        return result;
    }
    result.mode = Mode::New;

    const PipelineCapabilities *legacy = nullptr;
    const PipelineCapabilities *templated = nullptr;
    for (const auto &capability : input.pipelineCapabilities) {
        if (capability.pipeline == Pipeline::Legacy)
            legacy = &capability;
        else
            templated = &capability;
    }

    const auto &pipelineVar = config.get(CONFIG_PIPELINE);
    const Location pipelineSource = cliPipelines ? Location() : pipelineVar.location();
    const auto selected = filterIdentityTokens(cliPipelines ? input.cliPipelines
                                                          : pipelineVar.asStringList());
    if (selected.isEmpty())
        reporter.add(Diagnostic::Kind::Error, pipelineSource, u"Pipeline selection is empty."_s,
                     u"Select at least one pipeline in the configuration or on the command line."_s);
    for (const QString &name : selected) {
        const auto pipeline = pipelineFromName(name);
        if (!pipeline) {
            reporter.add(Diagnostic::Kind::Error, pipelineSource,
                         "Pipeline '%1' is not recognized."_L1.arg(name),
                         u"QDoc supports the 'legacy' and 'template' pipelines."_s);
            continue;
        }
        const auto *capability = *pipeline == Pipeline::Legacy ? legacy : templated;
        if (!capability || !capability->available) {
            reporter.add(Diagnostic::Kind::Error, pipelineSource,
                         "Pipeline '%1' is unavailable."_L1.arg(name));
            continue;
        }
        result.pipelines.push_back(*pipeline);
    }

    const auto &formatsVar = config.get(CONFIG_OUTPUTFORMATS);
    const auto configured = filterIdentityTokens(formatsVar.asStringList());
    const auto requested = input.cliFormats.isEmpty() ? configured
                                                    : filterIdentityTokens(input.cliFormats);
    if (requested.isEmpty())
        reporter.add(Diagnostic::Kind::Error,
                     input.cliFormats.isEmpty() ? formatsVar.location() : Location(),
                     u"Format selection is empty."_s,
                     u"Select at least one output format in the configuration or on the command line."_s);

    const QSet<QString> requestedSet(requested.cbegin(), requested.cend());
    const bool templateEnabled = result.pipelines.contains(Pipeline::Template);
    const bool legacyEnabled = result.pipelines.contains(Pipeline::Legacy);
    const auto declares = [](const PipelineCapabilities *capability, const QString &format) {
        return capability && (capability->supportedFormats.contains(format)
                              || capability->brokenFormats.contains(format));
    };
    const auto addFormat = [&](const QString &format, const Location &location, bool configured) {
        Format entry;
        entry.format = format;
        entry.requested = requestedSet.contains(format);
        entry.configured = configured;
        entry.location = location;
        if (!isValidFormatIdentity(format)) {
            reporter.add(Diagnostic::Kind::Error, location,
                         "Invalid format identity '%1'."_L1.arg(format),
                         u"Format names may contain letters, digits, and underscores only."_s);
        }

        const PipelineCapabilities *owner = nullptr;
        if (templateEnabled && declares(templated, format))
            owner = templated;
        else if (legacyEnabled && declares(legacy, format))
            owner = legacy;

        if (owner) {
            entry.pipeline = owner->pipeline;
            entry.fileExtension = owner->formatExtensions.value(format);
            if (entry.requested && owner->brokenFormats.contains(format)) {
                reporter.add(Diagnostic::Kind::Error, location,
                             "Format '%1' is declared by pipeline '%2' but the declaration "
                             "is broken."_L1.arg(format, pipelineName(owner->pipeline)),
                             u"QDoc does not fall back to another pipeline for a broken declaration."_s);
            } else {
                const QString extensionKey = configKey(format, u"extension"_s);
                const auto &extension = config.get(extensionKey);
                const bool overridden = config.contains(extensionKey);
                if (overridden) {
                    const auto values = extension.asStringList();
                    entry.fileExtension = values.value(0);
                    if (entry.requested && values.size() > 1)
                        reporter.add(Diagnostic::Kind::Error, extension.location(),
                                     "%1 must contain a single value."_L1.arg(extensionKey));
                }
                if (entry.requested && entry.fileExtension.isEmpty())
                    reporter.add(Diagnostic::Kind::Error,
                                 overridden ? extension.location() : location,
                                 "Format '%1' has no file extension."_L1.arg(format),
                                 "Provide a pipeline default or a non-empty %1."_L1.arg(extensionKey));
            }
        } else if (entry.requested) {
            QStringList names;
            for (Pipeline pipeline : result.pipelines)
                names.append(pipelineName(pipeline));
            reporter.add(Diagnostic::Kind::Error, location,
                         "Format '%1' is not supported by any selected pipeline."_L1.arg(format),
                         names.isEmpty() ? u"No pipeline is available."_s
                                         : "Selected pipelines: %1."_L1.arg(names.join(u", "_s)));
        }
        result.formats.push_back(std::move(entry));
    };

    for (const QString &format : configured)
        addFormat(format, formatsVar.location(), true);
    for (const QString &format : requested) {
        if (!configured.contains(format))
            addFormat(format, Location(), false);
    }
    return result;
}

/*!
    Returns \c true if any diagnostic blocks publication. Notices alone do not
    invalidate the selection or consume the documentation warning budget.
*/
bool OutputSelection::hasErrors() const
{
    for (const auto &diagnostic : diagnostics) {
        if (diagnostic.kind == Diagnostic::Kind::Error)
            return true;
    }
    return false;
}

/*!
    Returns the entry for the case-sensitive identity \a format, or \c{nullptr}
    if it is absent. The entry may be configured but unrequested.

    The result borrows storage from this selection and is valid only while that
    storage remains alive and unchanged. Looking up an entry does not validate it.
*/
const OutputSelection::Format *OutputSelection::findFormat(const QString &format) const
{
    for (const auto &entry : formats) {
        if (entry.format == format)
            return &entry;
    }
    return nullptr;
}

QT_END_NAMESPACE
