// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "templatepackage.h"

#include "config.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>

#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \struct TemplatePackageInput
    \internal
    \inmodule QDoc
    \brief Supplies engine facts that a template package resolution needs.

    The caller states which optional engine capabilities this QDoc build
    provides and where the built-in package lives. Production callers keep the
    embedded resource root; tests can point at a source checkout instead.
*/

/*!
    \variable TemplatePackageInput::engineCapabilities
    \brief The optional capability names this QDoc build can honor.

    A manifest that requires a capability absent from this set is an error.
    QDoc currently declares no optional capabilities, so the production set is
    empty until the engine grows an explicitly supported feature.
*/

/*!
    \variable TemplatePackageInput::builtinRoot
    \brief The directory holding the built-in \c{qdoc:default} package.

    The directory must contain a manifest like any other package. The default
    names the templates embedded in the QDoc executable.
*/

/*!
    \struct TemplatePackage
    \internal
    \inmodule QDoc
    \brief Resolves the effective template package declared by a configuration.

    A package is a directory holding a \c{qdoc-template.json} manifest. The
    manifest declares the manifest schema version, the template API version it
    was written against, an optional single parent to extend, optional engine
    capabilities it requires, and the formats it provides. Each format carries
    a default file extension and maps the page roles QDoc renders to template
    files. Paths are relative to the manifest that declares them.

    Inheritance is explicit and single-parent. A child inherits every parent
    format and role mapping and overrides only what it declares. Declaring a
    format as \c false drops it from the effective package. An override that
    names a missing file breaks the format rather than falling back to the
    parent's template. Lists do not concatenate: each manifest's requirements
    are validated on their own.

    The built-in package is addressed as \c{qdoc:default} and loads through
    the same code path as an external directory, so QDoc itself carries no
    branding. Resolution reads files but never writes, never renders, and
    never emits diagnostics; the caller presents errors before publication.
*/

/*!
    \typealias TemplatePackage::Diagnostic
    \internal
    \brief Uses the same diagnostic data and severity contract as OutputSelection.
*/

/*!
    \variable TemplatePackage::supportedSchemaVersion
    \brief The manifest syntax version this QDoc reads.

    Manifests must declare exactly this \c{schemaVersion}. A newer or older
    syntax is an error rather than a best-effort interpretation.
*/

/*!
    \variable TemplatePackage::providedTemplateApiVersion
    \brief The render-data contract version this QDoc offers templates.

    Manifests must declare exactly this \c{templateApiVersion}. A package
    written against another contract is rejected before any rendering.
*/

/*!
    \struct TemplatePackage::Template
    \internal
    \inmodule QDoc
    \brief Names one template file and the manifest that declared it.
*/

/*!
    \variable TemplatePackage::Template::path
    \brief The complete path of the template file, resolved against its manifest directory.

    For the built-in package this is a resource path.
*/

/*!
    \variable TemplatePackage::Template::location
    \brief The manifest that declared this mapping, which may be a parent package.
*/

/*!
    \struct TemplatePackage::Format
    \internal
    \inmodule QDoc
    \brief Describes one format the effective package provides.

    Extension and role mappings may come from different manifests in the
    inheritance chain; each value records its own origin. Values never leak
    between formats: a format that declares no extension has none, regardless
    of what sibling formats declare.
*/

/*!
    \variable TemplatePackage::Format::format
    \brief The case-sensitive publication identity, such as \c HTML.
*/

/*!
    \variable TemplatePackage::Format::extension
    \brief The default file extension without a separator, or empty if none was declared.

    OutputSelection applies this as the pipeline default and still honors an
    explicit \c{<Format>.extension} configuration override.
*/

/*!
    \variable TemplatePackage::Format::extensionLocation
    \brief The manifest that declared the effective extension, if any.
*/

/*!
    \variable TemplatePackage::Format::templates
    \brief The effective role to template mapping, keyed by role name.

    See roles() for the roles every format must map. Several roles may name
    the same file.
*/

/*!
    \variable TemplatePackage::Format::location
    \brief The most derived manifest that mentions this format.
*/

/*!
    \variable TemplatePackage::Format::broken
    \brief Whether the declaration is incomplete or names a missing template.

    A broken format remains an ownership claim for OutputSelection; it is an
    error to request it, and QDoc never substitutes another pipeline or an
    inherited template for it.
*/

/*!
    \variable TemplatePackage::id
    \brief The built-in identifier or the complete directory of the selected package.
*/

/*!
    \variable TemplatePackage::location
    \brief The \c{template.package} declaration, or empty for the implicit built-in package.
*/

/*!
    \variable TemplatePackage::loaded
    \brief Whether every manifest in the inheritance chain was read and validated.

    A loaded package can still carry broken formats; check hasErrors() for the
    combined verdict.
*/

/*!
    \variable TemplatePackage::searchPaths
    \brief The package directories from the most derived to the root package.

    Consumers resolve template includes against these directories in order, so
    a child can override a partial without redeclaring the roles that use it.
*/

/*!
    \variable TemplatePackage::formats
    \brief The effective formats in identity order.
*/

/*!
    \variable TemplatePackage::diagnostics
    \brief The errors collected while reading, validating, and merging manifests.
*/

namespace {

using Diagnostic = TemplatePackage::Diagnostic;

struct FormatDeclaration
{
    bool removed = false;
    bool hasExtension = false;
    QString extension;
    QMap<QString, QString> templates;
    Location location;
};

struct Manifest
{
    QString dir;
    Location location;
    QString extends;
    QMap<QString, FormatDeclaration> formats;
};

void report(QVector<Diagnostic> &diagnostics, const Location &location, QString message,
            QString details = {})
{
    diagnostics.push_back(
            Diagnostic{ Diagnostic::Kind::Error, location, std::move(message), std::move(details) });
}

int lineOfOffset(const QByteArray &text, int offset)
{
    return 1 + static_cast<int>(text.left(offset).count('\n'));
}

QString packageDirectory(const QString &reference, const QString &baseDir,
                         const QString &builtinRoot)
{
    if (reference == TemplatePackage::builtinId())
        return QDir::cleanPath(builtinRoot);
    if (QDir::isAbsolutePath(reference))
        return QDir::cleanPath(reference);
    return QDir::cleanPath(QDir(baseDir).absoluteFilePath(reference));
}

bool readVersion(const QJsonObject &object, const QString &key, int supported,
                 const QString &subject, const QString &verb, const Location &location,
                 QVector<Diagnostic> &diagnostics)
{
    const QJsonValue value = object.value(key);
    if (value.isUndefined()) {
        report(diagnostics, location, "Manifest key '%1' is required."_L1.arg(key));
        return false;
    }
    const int actual = value.isDouble() ? value.toInt(-1) : -1;
    if (actual < 0) {
        report(diagnostics, location,
               "Manifest key '%1' must be a non-negative integer."_L1.arg(key));
        return false;
    }
    if (actual != supported) {
        report(diagnostics, location, "%1 %2 is not supported."_L1.arg(subject).arg(actual),
               "This QDoc %1 %2 %3."_L1.arg(verb, subject.toLower()).arg(supported));
        return false;
    }
    return true;
}

bool readFormat(const QString &format, const QJsonValue &value, const QString &dir,
                const Location &location, QMap<QString, FormatDeclaration> &formats,
                QVector<Diagnostic> &diagnostics)
{
    FormatDeclaration declaration;
    declaration.location = location;
    if (value.isBool() && !value.toBool()) {
        declaration.removed = true;
        formats.insert(format, declaration);
        return true;
    }
    if (!value.isObject()) {
        report(diagnostics, location, "Format '%1' must be an object or false."_L1.arg(format),
               u"Use false to drop an inherited format."_s);
        return false;
    }

    bool ok = true;
    const QJsonObject entry = value.toObject();
    for (const QString &key : entry.keys()) {
        if (key != "extension"_L1 && key != "templates"_L1) {
            report(diagnostics, location,
                   "Unknown key '%1' for format '%2'."_L1.arg(key, format),
                   u"Supported keys: extension, templates."_s);
            ok = false;
        }
    }
    if (entry.contains("extension"_L1)) {
        const QJsonValue extension = entry.value("extension"_L1);
        const QString text = extension.isString() ? extension.toString() : QString();
        if (text.isEmpty() || text.contains(u'/') || text.contains(u'\\')) {
            report(diagnostics, location,
                   "Format '%1' declares an invalid or empty extension."_L1.arg(format),
                   u"Provide a non-empty file extension without path separators."_s);
            ok = false;
        } else {
            declaration.hasExtension = true;
            declaration.extension = text;
        }
    }
    if (entry.contains("templates"_L1)) {
        const QJsonValue templates = entry.value("templates"_L1);
        if (!templates.isObject()) {
            report(diagnostics, location,
                   "Format '%1' must map template roles in a 'templates' object."_L1.arg(format));
            ok = false;
        } else {
            const QStringList roles = TemplatePackage::roles();
            const QJsonObject mapping = templates.toObject();
            for (auto role = mapping.constBegin(); role != mapping.constEnd(); ++role) {
                if (!roles.contains(role.key())) {
                    report(diagnostics, location,
                           "Unknown template role '%1' for format '%2'."_L1.arg(role.key(), format),
                           "Supported roles: %1."_L1.arg(roles.join(u", "_s)));
                    ok = false;
                    continue;
                }
                const QString path = role.value().isString() ? role.value().toString() : QString();
                if (path.isEmpty()) {
                    report(diagnostics, location,
                           "Format '%1' declares an empty template for role '%2'."_L1
                                   .arg(format, role.key()));
                    ok = false;
                    continue;
                }
                declaration.templates.insert(role.key(),
                                             QDir::cleanPath(QDir(dir).absoluteFilePath(path)));
            }
        }
    }
    if (ok)
        formats.insert(format, declaration);
    return ok;
}

bool readManifest(const QString &dir, const Location &from, const QSet<QString> &capabilities,
                  Manifest &manifest, QVector<Diagnostic> &diagnostics)
{
    const QString manifestPath = dir + u'/' + TemplatePackage::manifestFileName();
    if (!QFileInfo(manifestPath).isFile()) {
        report(diagnostics, from,
               "Template package '%1' has no %2 manifest."_L1.arg(dir, TemplatePackage::manifestFileName()),
               u"A package is a directory containing a manifest."_s);
        return false;
    }
    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly)) {
        report(diagnostics, from,
               "Cannot read template package manifest '%1': %2"_L1.arg(manifestPath,
                                                                       file.errorString()));
        return false;
    }
    const QByteArray raw = file.readAll();
    const Location location(manifestPath);
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        Location at = location;
        at.setLineNo(lineOfOffset(raw, parseError.offset));
        report(diagnostics, at,
               "Cannot parse template package manifest: %1"_L1.arg(parseError.errorString()));
        return false;
    }
    if (!document.isObject()) {
        report(diagnostics, location,
               u"Template package manifest must contain a JSON object."_s);
        return false;
    }

    manifest.dir = dir;
    manifest.location = location;
    const QJsonObject object = document.object();
    bool ok = true;
    static const QSet<QString> knownKeys{ u"schemaVersion"_s, u"templateApiVersion"_s,
                                          u"extends"_s, u"requires"_s, u"formats"_s };
    for (const QString &key : object.keys()) {
        if (!knownKeys.contains(key)) {
            report(diagnostics, location, "Unknown manifest key '%1'."_L1.arg(key),
                   u"Supported keys: schemaVersion, templateApiVersion, extends, requires, formats."_s);
            ok = false;
        }
    }
    if (!readVersion(object, u"schemaVersion"_s, TemplatePackage::supportedSchemaVersion,
                     u"Manifest schema version"_s, u"supports"_s, location, diagnostics))
        return false;
    if (!readVersion(object, u"templateApiVersion"_s, TemplatePackage::providedTemplateApiVersion,
                     u"Template API version"_s, u"provides"_s, location, diagnostics))
        return false;

    if (object.contains("extends"_L1)) {
        const QJsonValue value = object.value("extends"_L1);
        if (!value.isString() || value.toString().isEmpty()) {
            report(diagnostics, location, u"Manifest key 'extends' must name a parent package."_s,
                   "Use a directory relative to this manifest or '%1'."_L1
                           .arg(TemplatePackage::builtinId()));
            ok = false;
        } else {
            manifest.extends = value.toString();
        }
    }
    if (object.contains("requires"_L1)) {
        const QJsonValue value = object.value("requires"_L1);
        if (!value.isArray()) {
            report(diagnostics, location,
                   u"Manifest key 'requires' must be an array of capability names."_s);
            ok = false;
        } else {
            const QJsonArray entries = value.toArray();
            for (const QJsonValue &entry : entries) {
                const QString name = entry.isString() ? entry.toString() : QString();
                if (name.isEmpty()) {
                    report(diagnostics, location,
                           u"Manifest key 'requires' must list non-empty capability names."_s);
                    ok = false;
                } else if (!capabilities.contains(name)) {
                    report(diagnostics, location,
                           "Template package requires capability '%1', which this QDoc does not provide."_L1
                                   .arg(name));
                    ok = false;
                }
            }
        }
    }
    if (object.contains("formats"_L1)) {
        const QJsonValue value = object.value("formats"_L1);
        if (!value.isObject()) {
            report(diagnostics, location,
                   u"Manifest key 'formats' must be an object keyed by format identity."_s);
            ok = false;
        } else {
            const QJsonObject formats = value.toObject();
            for (auto it = formats.constBegin(); it != formats.constEnd(); ++it) {
                if (!OutputSelection::isValidFormatIdentity(it.key())) {
                    report(diagnostics, location, "Invalid format identity '%1'."_L1.arg(it.key()),
                           u"Format names may contain letters, digits, and underscores only."_s);
                    ok = false;
                    continue;
                }
                if (!readFormat(it.key(), it.value(), dir, location, manifest.formats, diagnostics))
                    ok = false;
            }
        }
    }
    return ok;
}

} // namespace

/*!
    Returns the identifier of the package built into QDoc.
*/
QString TemplatePackage::builtinId()
{
    return u"qdoc:default"_s;
}

/*!
    Returns the file name a package directory must contain.
*/
QString TemplatePackage::manifestFileName()
{
    return u"qdoc-template.json"_s;
}

/*!
    Returns the roles every format must map to a template, in rendering order.

    The roles mirror the generator's entry points: a documentation page, a
    collection page, a C++ reference page, a QML type page, a member list, and
    an obsolete member list.
*/
QStringList TemplatePackage::roles()
{
    return { u"page"_s, u"collection"_s, u"cppref"_s, u"qmltype"_s, u"members"_s,
             u"obsolete"_s };
}

/*!
    Resolves the package that \a config selects through \c{template.package},
    or the built-in package when the setting is absent, using the engine facts
    in \a input.

    A relative package path is taken from the directory of the configuration
    file that declared it. Returns diagnostics as data; the caller must check
    hasErrors() before trusting formats or capabilities().
*/
TemplatePackage TemplatePackage::resolve(const Config &config, const TemplatePackageInput &input)
{
    TemplatePackage result;
    QString reference = builtinId();
    QString baseDir = config.currentDir();
    if (config.contains(CONFIG_TEMPLATEPACKAGE)) {
        const ConfigVar &var = config.get(CONFIG_TEMPLATEPACKAGE);
        result.location = var.location();
        const QStringList values = var.asStringList();
        if (values.size() > 1) {
            report(result.diagnostics, result.location,
                   "%1 must contain a single value."_L1.arg(CONFIG_TEMPLATEPACKAGE),
                   "Name one package directory or '%1'."_L1.arg(builtinId()));
            return result;
        }
        reference = values.value(0);
        if (reference.isEmpty()) {
            report(result.diagnostics, result.location,
                   "%1 is empty."_L1.arg(CONFIG_TEMPLATEPACKAGE),
                   "Name a package directory relative to the declaring configuration file, or '%1'."_L1
                           .arg(builtinId()));
            return result;
        }
        if (!var.path().isEmpty())
            baseDir = var.path();
    }

    QVector<Manifest> chain;
    QStringList visited;
    Location from = result.location;
    for (;;) {
        if (reference.startsWith("qdoc:"_L1) && reference != builtinId()) {
            report(result.diagnostics, from,
                   "Unknown built-in template package '%1'."_L1.arg(reference),
                   "QDoc provides '%1'."_L1.arg(builtinId()));
            return result;
        }
        const QString dir = packageDirectory(reference, baseDir, input.builtinRoot);
        if (chain.isEmpty())
            result.id = reference == builtinId() ? builtinId() : dir;
        if (visited.contains(dir)) {
            visited.append(dir);
            report(result.diagnostics, from,
                   "Template package inheritance cycle: %1."_L1.arg(visited.join(u" extends "_s)),
                   u"A package may extend one parent and may not extend itself, directly or indirectly."_s);
            return result;
        }
        visited.append(dir);
        Manifest manifest;
        if (!readManifest(dir, from, input.engineCapabilities, manifest, result.diagnostics))
            return result;
        chain.append(manifest);
        if (manifest.extends.isEmpty())
            break;
        reference = manifest.extends;
        baseDir = manifest.dir;
        from = manifest.location;
    }
    result.loaded = true;
    for (const Manifest &manifest : std::as_const(chain))
        result.searchPaths.append(manifest.dir);

    QMap<QString, Format> effective;
    for (auto manifest = chain.crbegin(); manifest != chain.crend(); ++manifest) {
        for (auto it = manifest->formats.constBegin(); it != manifest->formats.constEnd(); ++it) {
            const FormatDeclaration &declaration = it.value();
            if (declaration.removed) {
                effective.remove(it.key());
                continue;
            }
            Format &format = effective[it.key()];
            format.format = it.key();
            format.location = declaration.location;
            if (declaration.hasExtension) {
                format.extension = declaration.extension;
                format.extensionLocation = declaration.location;
            }
            for (auto role = declaration.templates.constBegin();
                 role != declaration.templates.constEnd(); ++role) {
                format.templates.insert(role.key(), Template{ role.value(), declaration.location });
            }
        }
    }

    const QStringList allRoles = roles();
    for (Format &format : effective) {
        for (const QString &role : allRoles) {
            if (!format.templates.contains(role)) {
                report(result.diagnostics, format.location,
                       "Format '%1' declares no template for role '%2'."_L1.arg(format.format, role),
                       u"Declare every role or inherit it from a parent package."_s);
                format.broken = true;
            }
        }
        for (auto it = format.templates.constBegin(); it != format.templates.constEnd(); ++it) {
            if (!QFileInfo(it.value().path).isFile()) {
                report(result.diagnostics, it.value().location,
                       "Template '%1' for role '%2' of format '%3' does not exist."_L1
                               .arg(it.value().path, it.key(), format.format),
                       u"QDoc does not fall back to an inherited template for a failed override."_s);
                format.broken = true;
            }
        }
        result.formats.append(format);
    }
    return result;
}

/*!
    Returns \c true if any diagnostic blocks use of the package.
*/
bool TemplatePackage::hasErrors() const
{
    for (const auto &diagnostic : diagnostics) {
        if (diagnostic.kind == Diagnostic::Kind::Error)
            return true;
    }
    return false;
}

/*!
    Returns the effective entry for the case-sensitive identity \a format, or
    \c{nullptr} if the package does not provide it.

    The pointer borrows storage from this package and remains valid only while
    that storage is alive and unchanged. A returned entry may be broken.
*/
const TemplatePackage::Format *TemplatePackage::findFormat(const QString &format) const
{
    for (const auto &entry : formats) {
        if (entry.format == format)
            return &entry;
    }
    return nullptr;
}

/*!
    Returns the template pipeline's capabilities as OutputSelection expects them.

    The pipeline is available when every manifest loaded. Complete formats are
    supported, broken formats remain declared ownership claims, and declared
    extensions become the pipeline defaults.
*/
PipelineCapabilities TemplatePackage::capabilities() const
{
    PipelineCapabilities result;
    result.pipeline = Pipeline::Template;
    result.available = loaded;
    for (const Format &format : formats) {
        if (format.broken)
            result.brokenFormats.insert(format.format);
        else
            result.supportedFormats.insert(format.format);
        if (!format.extension.isEmpty())
            result.formatExtensions.insert(format.format, format.extension);
    }
    return result;
}

QT_END_NAMESPACE
