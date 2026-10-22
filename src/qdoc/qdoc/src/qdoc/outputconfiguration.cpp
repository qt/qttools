// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "outputconfiguration.h"

#include "config.h"

#include <QtCore/qdir.h>
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

bool isValidModuleDir(const QString &project)
{
    static const QRegularExpression unsafe(uR"([/\\<>:"|?*\x00-\x1F])"_s);
    return !project.isEmpty() && project != "."_L1 && project != ".."_L1
            && project == project.trimmed() && !project.endsWith(u'.')
            && !unsafe.match(project).hasMatch();
}

std::optional<PublicationLayout> parsePublicationLayout(const QString &value)
{
    if (value == "flat"_L1)
        return PublicationLayout::Flat;
    if (value == "per-module"_L1)
        return PublicationLayout::PerModule;
    return std::nullopt;
}

QString joinPath(const QString &base, const QString &sub)
{
    if (sub.isEmpty())
        return base.isEmpty() ? u"."_s : QDir::cleanPath(base);
    if (base.isEmpty())
        return QDir::cleanPath(sub);
    return QDir::cleanPath(base + u'/' + sub);
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

/*!
    \enum PublicationLayout
    \internal
    \inmodule QDoc
    \relates OutputConfiguration
    \brief Determines whether a format's publication tree has a module directory.

    Layout is independent of execution mode and format directory selection.

    \value Flat Modules can share the format directory. Image and style
                subdirectories remain possible.
    \value PerModule Append the lower-case project identity to the format directory.
*/

/*!
    \struct OutputConfiguration
    \internal
    \inmodule QDoc
    \brief Adds publication destinations to a resolved output selection.

    Resolution consumes OutputSelection without constructing generators, parsing
    source, creating output, or emitting diagnostics. Compatibility mode remains
    a marker for the unchanged legacy interpretation, not a new layout plan.

    The first configured format defaults to the publication root. Later
    configured formats and CLI-only formats default to lower-case format names.
    Explicit \c{<Format>.outputsubdir} values apply literally, including empty
    and \c{../} values. Replacing requested formats or moving the first format
    does not promote another configured format into the root.

    Layout precedence is \c{<Format>.publicationlayout}, \c{publicationlayout},
    the explicit deprecated \c{<Format>.nosubdirs} alias, then Flat. The alias
    maps true to Flat and false, including an empty declaration, to PerModule.
    Invalid or empty new layout values are errors, not a request to use an
    inherited alias. PerModule requires a safe module directory component.

    Equal normalized destinations for requested formats are errors; nested
    trees are allowed. Before writing, consumers must also validate filesystem
    aliases and case, reserved subtrees, and actual artifact ownership. Lexical
    planning neither performs those checks nor authorizes filesystem writes.
*/

/*!
    \typealias OutputConfiguration::Mode
    \internal
    \brief Uses the same interpretation modes as OutputSelection.
*/

/*!
    \typealias OutputConfiguration::Diagnostic
    \internal
    \brief Uses the same diagnostic data and severity contract as OutputSelection.
*/

/*!
    \struct OutputConfiguration::FormatPlan
    \internal
    \inmodule QDoc
    \brief Extends a format selection with its publication directory and layout.

    The inherited request and ownership fields retain their OutputSelection
    meaning. A non-requested entry preserves configured assignment but is not
    another publication. Check the containing configuration's errors before
    using a requested plan.
*/

/*!
    \variable OutputConfiguration::FormatPlan::formatDir
    \brief The literal or default format directory relative to the publication root.

    Empty means the root. Parent components are deliberate, not an instruction
    to rewrite the destination to remain under the root.
*/

/*!
    \variable OutputConfiguration::FormatPlan::formatDirLocation
    \brief The directory declaration, or the format-list origin for a default directory.

    CLI-only defaults and programmatic declarations can have an empty location.
*/

/*!
    \variable OutputConfiguration::FormatPlan::layout
    \brief The resolved policy for adding a module directory.
*/

/*!
    \variable OutputConfiguration::FormatPlan::layoutLocation
    \brief The effective layout or deprecated-alias declaration, if one applies.

    This is empty for the implicit Flat default or absent source provenance.
*/

/*!
    \variable OutputConfiguration::FormatPlan::moduleDir
    \brief The lower-case project directory for PerModule, or empty for Flat.

    An invalid PerModule identity leaves this empty and produces an error for
    a requested format. It must not be interpreted as a valid Flat fallback.
*/

/*!
    \variable OutputConfiguration::mode
    \brief The interpretation mode inherited from output selection.
*/

/*!
    \variable OutputConfiguration::pipelines
    \brief The available enabled pipelines inherited from output selection.
*/

/*!
    \variable OutputConfiguration::root
    \brief The publication root before complete-path normalization.

    The invocation's output-directory override wins over configured
    \c{outputdir}. Empty means the current directory. The override replaces only
    the root, not the format or module layout.
*/

/*!
    \variable OutputConfiguration::formats
    \brief The configured and CLI-only format plans in selection order.

    Only entries whose inherited \c{requested} value is true claim destinations.
*/

/*!
    \variable OutputConfiguration::diagnostics
    \brief The combined, deduplicated selection and layout diagnostics.

    These records have no emission or warning-budget side effects.
*/

/*!
    Resolves selection and destination layout from \a config and \a input.

    Also reads the invocation's output-directory override from Config. Returns
    a compatibility marker without destinations when the new model is not
    selected. Errors remain data; the caller must reject an invalid plan before
    any publication writes.
*/
OutputConfiguration OutputConfiguration::resolve(const Config &config,
                                                 const OutputConfigurationInput &input)
{
    auto selection = OutputSelection::resolve(config, input);
    OutputConfiguration result;
    result.mode = selection.mode;
    result.pipelines = std::move(selection.pipelines);
    result.diagnostics = std::move(selection.diagnostics);
    if (result.mode == Mode::Compatibility)
        return result;

    DiagnosticReporter reporter(result.diagnostics);
    result.root = Config::overrideOutputDir.isNull() ? config.get(CONFIG_OUTPUTDIR).asString()
                                                   : Config::overrideOutputDir;
    const auto &project = config.get(CONFIG_PROJECT);
    const QString moduleDir = project.asString().toLower();
    for (qsizetype i = 0; i < selection.formats.size(); ++i) {
        FormatPlan plan;
        static_cast<OutputSelection::Format &>(plan) = std::move(selection.formats[i]);
        const QString subKey = configKey(plan.format, u"outputsubdir"_s);
        if (config.contains(subKey)) {
            const auto &sub = config.get(subKey);
            const auto values = sub.asStringList();
            if (values.size() > 1)
                reporter.add(Diagnostic::Kind::Error, sub.location(),
                             "%1 must contain a single value."_L1.arg(subKey),
                             u"Use one path per format."_s);
            plan.formatDir = values.value(0);
            plan.formatDirLocation = sub.location();
        } else {
            plan.formatDir = plan.configured && i == 0 ? QString() : plan.format.toLower();
            plan.formatDirLocation = plan.location;
        }

        const QString perKey = configKey(plan.format, CONFIG_PUBLICATIONLAYOUT);
        const QString layoutKey = config.contains(perKey) ? perKey : CONFIG_PUBLICATIONLAYOUT;
        if (config.contains(layoutKey)) {
            const auto &value = config.get(layoutKey);
            plan.layoutLocation = value.location();
            if (const auto layout = parsePublicationLayout(value.asString())) {
                plan.layout = *layout;
            } else {
                reporter.add(Diagnostic::Kind::Error, value.location(),
                             "Invalid %1 '%2'."_L1.arg(layoutKey, value.asString()),
                             u"Expected 'flat' or 'per-module'."_s);
            }
        } else {
            const QString aliasKey = configKey(plan.format, u"nosubdirs"_s);
            if (config.contains(aliasKey)) {
                const auto &alias = config.get(aliasKey);
                plan.layout = alias.asBool() ? PublicationLayout::Flat : PublicationLayout::PerModule;
                plan.layoutLocation = alias.location();
                reporter.add(Diagnostic::Kind::Notice, alias.location(),
                             "%1 is deprecated; use %2 or publicationlayout instead."_L1
                                     .arg(aliasKey, perKey));
            }
        }
        if (plan.layout == PublicationLayout::PerModule) {
            if (isValidModuleDir(moduleDir)) {
                plan.moduleDir = moduleDir;
            } else if (plan.requested) {
                reporter.add(Diagnostic::Kind::Error, project.location(),
                             "Invalid project identity '%1' for per-module publication."_L1
                                     .arg(project.asString()),
                             u"Project must be a safe directory name."_s);
            }
        }
        result.formats.push_back(std::move(plan));
    }

    QHash<QString, const FormatPlan *> destinations;
    for (const auto &plan : result.formats) {
        if (!plan.requested)
            continue;
        const QString dir = result.destination(plan);
        const auto it = destinations.constFind(dir);
        if (it == destinations.constEnd()) {
            destinations.insert(dir, &plan);
        } else {
            const auto &otherLocation = it.value()->formatDirLocation;
            reporter.add(Diagnostic::Kind::Error, plan.formatDirLocation,
                         "Conflicting output destinations for '%1' and '%2': %3"_L1
                                 .arg(it.value()->format, plan.format, dir),
                         otherLocation.isEmpty() ? QString()
                                 : "Other destination declared at %1."_L1.arg(otherLocation.toString()));
        }
    }
    return result;
}

/*!
    Returns \c true if a selection or layout diagnostic blocks publication.
    Migration notices alone do not invalidate the configuration.
*/
bool OutputConfiguration::hasErrors() const
{
    for (const auto &diagnostic : diagnostics) {
        if (diagnostic.kind == Diagnostic::Kind::Error)
            return true;
    }
    return false;
}

/*!
    Returns the plan for the case-sensitive identity \a format, or \c{nullptr}
    if absent. A configured entry can be present without being requested.

    The pointer borrows storage from this configuration and remains valid only
    while that storage is alive and unchanged. It does not certify plan validity.
*/
const OutputConfiguration::FormatPlan *OutputConfiguration::findFormat(const QString &format) const
{
    for (const auto &plan : formats) {
        if (plan.format == format)
            return &plan;
    }
    return nullptr;
}

/*!
    Returns the normalized root, format directory, and module directory for
    \a plan. Use an entry from this configuration after checking hasErrors().

    A root-only path is normalized too; \c{.} denotes the current directory.
    This query does not create directories, require their existence, resolve
    filesystem aliases, or check whether the entry is requested.
*/
QString OutputConfiguration::destination(const FormatPlan &plan) const
{
    return joinPath(root, joinPath(plan.formatDir, plan.moduleDir));
}

QT_END_NAMESPACE
