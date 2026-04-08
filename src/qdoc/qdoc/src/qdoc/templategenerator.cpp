// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "templategenerator.h"

#include "aggregate.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "documentationtraverser.h"
#include "filedocumentwriter.h"
#include "documentwriter.h"
#include "generator.h"
#include "hrefresolver.h"
#include "injabridge.h"
#include "ir/builder.h"
#include "ir/document.h"
#include "linkresolver.h"
#include "node.h"
#include "nodeextractor.h"
#include "outputcontext.h"
#include "outputproducerregistry.h"
#include "pagenode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"
#include "tree.h"
#include "utilities.h"

#include <utility>

#include <QtCore/qdir.h>
#include <QtCore/qdiriterator.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtextstream.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQDocTemplateGenerator, "qt.qdoc.templategenerator")

using namespace Qt::Literals;

static QString nodeTypeKey(const Node *node);
static void resolveDocumentLinks(LinkResolver *resolver, IR::Document &ir, const Node *relative);

/*!
    \class TemplateGenerator
    \internal
    \brief Generates documentation using external templates and a pre-built IR.

    TemplateGenerator implements OutputProducer and DocumentationHandler to
    generate documentation without inheriting from Generator. It uses
    DocumentationTraverser for tree traversal and delegates content generation
    to templates via the IR system.

    \section1 Architecture

    The generator follows a composition-based design:
    \list
        \li \b{OutputProducer}: Lifecycle interface (prepare/produce/finalize).
        \li \b{DocumentationHandler}: Content generation callbacks for
            traverser.
        \li \b{DocumentationTraverser}: Shared tree traversal logic.
        \li \b{DocumentWriter}: Output abstraction (file or string for tests).
    \endlist

    \sa DocumentationTraverser, DocumentationHandler, OutputProducer,
        IR::Builder
*/

TemplateGenerator::TemplateGenerator(FileResolver &fileResolver, QDocDatabase &qdb,
                                     const QString &format)
    : m_fileResolver(fileResolver)
    , m_qdb(qdb)
    , m_format(format.isEmpty() ? u"template"_s : format)
{
    OutputProducerRegistry::instance().registerProducer(this);
}

TemplateGenerator::~TemplateGenerator()
{
    OutputProducerRegistry::instance().unregisterProducer(this);
}

void TemplateGenerator::prepare()
{
    createDefaultWriter();

    const Config &config = Config::instance();

    QString extensionConfig = config.get(m_format + ".extension"_L1).asString();
    if (!extensionConfig.isEmpty())
        m_fileExtension = extensionConfig;

    QString templateDirConfig = config.get(m_format + ".templatedir"_L1).asString();

    if (templateDirConfig.isEmpty()) {
        m_templateDir.clear();
    } else if (QDir::isAbsolutePath(templateDirConfig)) {
        m_templateDir = templateDirConfig;
    } else {
        // Relative path: resolve relative to the qdocconf file's directory,
        // consistent with how sourcedirs, headerdirs, etc. are resolved.
        m_templateDir = QDir::cleanPath(
            QDir(config.currentDir()).absoluteFilePath(templateDirConfig));
    }

    bool foundTemplates = false;
    if (!m_templateDir.isEmpty()) {
        QDir templateDir(m_templateDir);
        if (templateDir.exists() && !templateDir.entryList(QDir::Files).isEmpty()) {
            foundTemplates = true;
            qCInfo(lcQDocTemplateGenerator) << "[%1]"_L1.arg(m_format) << "Using template directory:" << m_templateDir;
        } else if (!templateDir.exists()) {
            qCInfo(lcQDocTemplateGenerator)
                    << "[%1]"_L1.arg(m_format) << "Configured template directory does not exist:" << m_templateDir
                    << "- will use embedded templates";
        } else {
            qCInfo(lcQDocTemplateGenerator)
                    << "[%1]"_L1.arg(m_format) << "Configured template directory is empty:" << m_templateDir
                    << "- will use embedded templates";
        }
    } else {
        qCInfo(lcQDocTemplateGenerator)
                << "[%1]"_L1.arg(m_format) << "No external template directory configured - will use embedded templates";
    }

    if (!foundTemplates)
        m_templateDir.clear();

    m_emitStylesheet = config.get(m_format + ".stylesheet"_L1).asBool();

    m_stylesheetName = config.get(m_format + ".stylesheetname"_L1).asString();
    if (m_stylesheetName.isEmpty())
        m_stylesheetName = u"qdoc-default.css"_s;

    // Enforce plain filename — no directory separators, no traversal.
    // The stylesheet is copied to and referenced from the output root,
    // so subpaths would create inconsistencies between theme-provided
    // assets and the QRC fallback.
    if (m_stylesheetName.contains('/'_L1)
        || m_stylesheetName.contains('\\'_L1)
        || m_stylesheetName.contains(".."_L1)
        || QDir::isAbsolutePath(m_stylesheetName)) {
        qCWarning(lcQDocTemplateGenerator)
            << "[%1]"_L1.arg(m_format) << "Ignoring stylesheetname:" << m_stylesheetName
            << "— must be a plain filename (no path separators)";
        m_stylesheetName = u"qdoc-default.css"_s;
    }

    copyAssets();

    // Construct link resolvers using OutputContext data.
    // TemplateGenerator doesn't inherit from Generator, so we use
    // m_context (OutputContext) which has the same prefix/suffix data.
    HrefResolverConfig hrefConfig;
    hrefConfig.project = m_context->project;
    hrefConfig.fileExtension = m_fileExtension;
    hrefConfig.useOutputSubdirs = m_context->useSubdirs;
    hrefConfig.noLinkErrors = Generator::noLinkErrors();
    hrefConfig.inclusionPolicy = config.createInclusionPolicy();
    hrefConfig.outputPrefixFn = [this](const Node *node) {
        return m_context->outputPrefix(nodeTypeKey(node));
    };
    hrefConfig.outputSuffixFn = [this](const Node *node) {
        return m_context->outputSuffix(nodeTypeKey(node));
    };
    hrefConfig.cleanRefFn = [](const QString &ref) { return Generator::cleanRef(ref); };
    hrefConfig.qmlTypeContextFn = []() -> const QmlTypeNode * {
        return Generator::qmlTypeContext();
    };
    m_hrefResolver = std::make_unique<HrefResolver>(hrefConfig);

    LinkResolverConfig linkConfig;
    linkConfig.autolinkErrors = Generator::autolinkErrors();
    linkConfig.noLinkErrors = Generator::noLinkErrors();
    m_linkResolver = std::make_unique<LinkResolver>(&m_qdb, *m_hrefResolver, linkConfig);
}

void TemplateGenerator::produce()
{
    DocumentationTraverser traverser;
    Node *root = m_qdb.primaryTreeRoot();
    if (root)
        traverser.traverse(root, *this);
}

void TemplateGenerator::finalize()
{
    m_writer.reset();
}

QString TemplateGenerator::format() const
{
    return m_format;
}

void TemplateGenerator::beginDocument(const QString &outputFileName)
{
    if (m_writer)
        m_writer->beginDocument(outputFileName);
}

void TemplateGenerator::endDocument()
{
    if (m_writer)
        m_writer->endDocument();
}

QString TemplateGenerator::fileName(const Node *node) const
{
    if (!node->url().isEmpty())
        return node->url();

    // Special case for simple page nodes (\page commands) with explicit
    // non-.html extensions. Use the normalized fileBase() but preserve
    // user specified extension
    if (node->isTextPageNode() && !node->isCollectionNode()) {
        QFileInfo originalName(node->name());
        QString suffix = originalName.suffix();
        if (!suffix.isEmpty() && suffix != "html"_L1) {
            // User specified a non-.html extension - use normalized base + original extension
            QString name = fileBase(node);
            return name + '.'_L1 + suffix;
        }
    }

    QString name = fileBase(node) + '.'_L1;
    return name + m_fileExtension;
}

void TemplateGenerator::generateCollectionNode(CollectionNode *cn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    IR::PageMetadata pm = NodeExtractor::extractPageMetadata(cn, m_hrefResolver.get());

    IR::Builder builder;
    IR::Document ir = builder.buildPageIR(std::move(pm));

    if (m_linkResolver)
        resolveDocumentLinks(m_linkResolver.get(), ir, cn);

    resolveImagePaths(ir);
    renderDocument(ir, "collection"_L1);
}

void TemplateGenerator::generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    IR::PageMetadata pm = NodeExtractor::extractPageMetadata(cn, m_hrefResolver.get());

    IR::Builder builder;
    IR::Document ir = builder.buildPageIR(std::move(pm));

    if (m_linkResolver)
        resolveDocumentLinks(m_linkResolver.get(), ir, cn);

    resolveImagePaths(ir);
    renderDocument(ir, "collection"_L1);
}

void TemplateGenerator::generatePageNode(PageNode *pn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    IR::PageMetadata pm = NodeExtractor::extractPageMetadata(pn, m_hrefResolver.get());

    IR::Builder builder;
    IR::Document ir = builder.buildPageIR(std::move(pm));

    if (m_linkResolver)
        resolveDocumentLinks(m_linkResolver.get(), ir, pn);

    resolveImagePaths(ir);
    renderDocument(ir, "page"_L1);
}

void TemplateGenerator::generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker)
{
    Q_UNUSED(marker);

    // Placeholder - IR integration pending
    if (m_writer && m_writer->isOpen()) {
        m_writer->writeLine(QString(u"<!-- TemplateGenerator: C++ Reference Page for "_s
                           + aggregate->name() + u" -->"_s));
        m_writer->writeLine(QString(u"<h1>"_s + aggregate->fullTitle() + u"</h1>"_s));
        m_writer->writeLine(u"<p>Template-based output (IR integration pending)</p>"_s);
    }
}

void TemplateGenerator::generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    IR::PageMetadata pm = NodeExtractor::extractPageMetadata(qcn, m_hrefResolver.get());
    auto allMembers = NodeExtractor::extractAllMembersIR(qcn, m_hrefResolver.get());

    IR::Builder builder;
    IR::Document ir = builder.buildPageIR(std::move(pm));

    if (allMembers)
        ir.membersPageUrl = fileBase(qcn) + "-members."_L1 + m_fileExtension;

    if (m_linkResolver)
        resolveDocumentLinks(m_linkResolver.get(), ir, qcn);

    resolveImagePaths(ir);
    renderDocument(ir, "qmltype"_L1);

    if (allMembers)
        generateMemberListingPage(qcn, *allMembers);
}

void TemplateGenerator::generateProxyPage(Aggregate *aggregate, CodeMarker *marker)
{
    Q_UNUSED(marker);

    // Placeholder - IR integration pending
    if (m_writer && m_writer->isOpen()) {
        m_writer->writeLine(QString(u"<!-- TemplateGenerator: Proxy Page for "_s
                           + aggregate->name() + u" -->"_s));
        m_writer->writeLine(QString(u"<h1>"_s + aggregate->fullTitle() + u"</h1>"_s));
        m_writer->writeLine(u"<p>Template-based output (IR integration pending)</p>"_s);
    }
}

void TemplateGenerator::mergeCollections(CollectionNode *cn)
{
    m_qdb.mergeCollections(cn);
}

QString TemplateGenerator::fileExtension() const
{
    return m_fileExtension;
}

/*!
    \internal
    Render phase: Format pre-built IR according to a template.

    This is TemplateGenerator's core responsibility. It receives IR and
    produces formatted output without any knowledge of Nodes or the database.
*/
void TemplateGenerator::renderDocument(const IR::Document &ir, const QString &templateBaseName)
{
    const QString templateFileName = templateBaseName + '.'_L1 + m_fileExtension;
    QString templateContent;

    if (!m_templateDir.isEmpty()) {
        QString templatePath = m_templateDir + '/'_L1 + templateFileName;
        QFile templateFile(templatePath);

        if (templateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            templateContent = QString::fromUtf8(templateFile.readAll());
            templateFile.close();
        }
    }

    if (templateContent.isEmpty()) {
        QFile resourceFile(":/qdoc/templates/"_L1 + templateFileName);
        if (resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            templateContent = QString::fromUtf8(resourceFile.readAll());
            resourceFile.close();
        }
    }

    if (templateContent.isEmpty())
        qFatal("TemplateGenerator[%s]: No template file found for extension '%s'. "
               "Ensure '%s.%s' exists in the configured template directory or in resources.",
               qPrintable(m_format), qPrintable(m_fileExtension),
               qPrintable(templateBaseName), qPrintable(m_fileExtension));

    QJsonObject json = ir.toJson();
    json["stylesheetEnabled"_L1] = m_emitStylesheet;
    json["stylesheetName"_L1] = m_stylesheetName;

    auto includeCallback = [this](const QString &name) { return resolveInclude(name); };
    QString rendered = InjaBridge::render(templateContent, json, includeCallback);

    if (m_writer && m_writer->isOpen())
        m_writer->write(rendered);
}

/*!
    \internal
    Render a raw QJsonObject through a named template.

    Unlike renderDocument(), this takes an arbitrary JSON object rather
    than an IR::Document. It's used for sub-pages (such as the member
    listing page) where the data structure differs from Document's.
*/
void TemplateGenerator::renderJson(const QJsonObject &json, const QString &templateBaseName)
{
    const QString templateFileName = templateBaseName + '.'_L1 + m_fileExtension;
    QString templateContent;

    if (!m_templateDir.isEmpty()) {
        QString templatePath = m_templateDir + '/'_L1 + templateFileName;
        QFile templateFile(templatePath);

        if (templateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            templateContent = QString::fromUtf8(templateFile.readAll());
            templateFile.close();
        }
    }

    if (templateContent.isEmpty()) {
        QFile resourceFile(":/qdoc/templates/"_L1 + templateFileName);
        if (resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            templateContent = QString::fromUtf8(resourceFile.readAll());
            resourceFile.close();
        }
    }

    if (templateContent.isEmpty())
        qFatal("TemplateGenerator[%s]: No template file found for '%s'. "
               "Ensure '%s.%s' exists in the configured template directory or in resources.",
               qPrintable(m_format), qPrintable(templateBaseName),
               qPrintable(templateBaseName), qPrintable(m_fileExtension));

    QJsonObject enrichedJson = json;
    enrichedJson["stylesheetEnabled"_L1] = m_emitStylesheet;
    enrichedJson["stylesheetName"_L1] = m_stylesheetName;

    auto includeCallback = [this](const QString &name) { return resolveInclude(name); };
    QString rendered = InjaBridge::render(templateContent, enrichedJson, includeCallback);

    if (m_writer && m_writer->isOpen())
        m_writer->write(rendered);
}

/*!
    \internal
    Generate a member listing sub-page for a C++ class or QML type.

    Opens a new output file, renders the all-members data through the
    \c members template, and closes the file. This mirrors
    HtmlGenerator::generateAllMembersFile() but uses the IR pipeline
    and template system instead of inline HTML generation.
*/
void TemplateGenerator::generateMemberListingPage(const Node *node,
                                                   const IR::AllMembersIR &allMembers)
{
    const QString membersFileName =
        fileBase(node) + "-members."_L1 + m_fileExtension;

    QJsonObject json = allMembers.toJson();
    json["title"_L1] = QString("List of All Members for "_L1 + allMembers.typeName);

    beginDocument(membersFileName);
    renderJson(json, "members"_L1);
    endDocument();
}

/*!
    \internal
    Resolves an Inja \c{{% include %}} directive to template content.

    Searches the filesystem first (for user-customized templates) and then
    Qt's embedded resource system (for bundled defaults). This enables
    Inja's include mechanism to work with Qt resources, where
    \c{std::ifstream} can't open \c{:/} paths.

    Returns the file content as a QString, or an empty QString if the file
    isn't found in either location.
*/
QString TemplateGenerator::resolveInclude(const QString &name) const
{
    if (!m_templateDir.isEmpty()) {
        QFile file(m_templateDir + '/'_L1 + name);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString::fromUtf8(file.readAll());
    }

    QFile resourceFile(":/qdoc/templates/"_L1 + name);
    if (resourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString::fromUtf8(resourceFile.readAll());

    return {};
}

/*!
    \internal
    Returns the output prefix/suffix key for a node based on its genus.

    This is used to look up configured prefixes and suffixes from OutputContext.
*/
static QString nodeTypeKey(const Node *node)
{
    if (node->isPageNode()) {
        switch (node->genus()) {
        case Genus::QML:
            return u"QML"_s;
        case Genus::CPP:
            return u"CPP"_s;
        default:
            break;
        }
    }
    return QString();
}

static void resolveDocumentLinks(LinkResolver *resolver, IR::Document &ir, const Node *relative)
{
    if (!ir.body.isEmpty())
        resolver->resolve(ir.body, relative);
    for (auto &section : ir.detailSections) {
        for (auto &member : section.members) {
            if (!member.body.isEmpty())
                resolver->resolve(member.body, relative);
            if (!member.alsoList.isEmpty())
                resolver->resolve(member.alsoList, relative);
        }
    }
}

/*!
    \internal
    Computes the base filename for a node, delegating the core computation
    to Utilities::computeFileBase().

    This handles caching and adapts the TemplateGenerator's OutputContext-based
    prefix/suffix lookup to the shared interface.
*/
QString TemplateGenerator::fileBase(const Node *node) const
{
    if (!node->isPageNode() && !node->isCollectionNode())
        node = node->parent();

    if (node->hasFileNameBase())
        return node->fileNameBase();

    QString result = Utilities::computeFileBase(
        node, m_context->project,
        [this](const Node *n) -> QString {
            if (n->isCollectionNode())
                return {};
            return m_context->outputPrefix(nodeTypeKey(n));
        },
        [this](const Node *n) { return m_context->outputSuffix(nodeTypeKey(n)); });

    const_cast<Node *>(node)->setFileNameBase(result);
    return result;
}

/*!
    \internal
    Creates the production FileDocumentWriter.

    This is called during initialization to create the default writer
    that writes to the filesystem. For testing, a mock writer can be
    injected by setting m_writer before calling prepare().

    Also initializes m_context with configuration values, enabling
    OutputContext-based access to output settings.
*/
void TemplateGenerator::createDefaultWriter()
{
    // Initialize OutputContext from configuration
    const Config &config = Config::instance();
    m_context.emplace(OutputContext::fromConfig(config, format()));

    if (m_writer)
        return; // Writer already set (e.g., injected for testing)

    m_writer = std::make_unique<FileDocumentWriter>(*m_context);
}

/*!
    \internal
    Walks content blocks recursively, resolving image filenames and copying
    image files to the output directory.

    For each InlineContent with type Image, the method resolves the raw
    filename through FileResolver, copies the file to output/images/, and
    prefixes the href with the images subdirectory path so that templates
    render correct src attributes.

*/
void TemplateGenerator::resolveImagePaths(IR::Document &ir)
{
    if (!m_context)
        return;

    const QString &outDir = m_context->outputDir.path();
    if (outDir.isEmpty())
        return;

    const QString imagesDir = u"images"_s;
    const QString imagesDestDir = outDir + '/'_L1 + imagesDir;

    QDir().mkpath(imagesDestDir);

    auto resolveInlines = [&](QList<IR::InlineContent> &inlines) {
        for (auto &inline_ : inlines) {
            if (inline_.type != IR::InlineType::Image)
                continue;

            auto resolved = m_fileResolver.resolve(inline_.href);
            if (!resolved)
                continue;

            const QString fileName = QFileInfo(resolved->get_path()).fileName();
            QFile::copy(resolved->get_path(), imagesDestDir + '/'_L1 + fileName);
            inline_.href = imagesDir + '/'_L1 + fileName;
        }
    };

    std::function<void(QList<IR::ContentBlock> &)> walkBlocks;
    walkBlocks = [&](QList<IR::ContentBlock> &blocks) {
        for (auto &block : blocks) {
            resolveInlines(block.inlineContent);
            if (!block.children.isEmpty())
                walkBlocks(block.children);
        }
    };

    walkBlocks(ir.body);

    for (auto &section : ir.detailSections) {
        for (auto &member : section.members) {
            walkBlocks(member.body);
            walkBlocks(member.alsoList);
        }
    }
}

/*!
    \internal
    Copies static assets to the output directory using a two-tier resolution
    strategy: templatedir assets take priority, with QRC defaults as fallback.

    When a template directory provides an \c{assets/} subdirectory, all files
    within it are copied recursively to the output directory, preserving the
    subdirectory structure. This supports fonts, images, and other static
    resources organized in subdirectories.

    After copying theme assets, the method checks whether a stylesheet is
    needed (controlled by \c{m_emitStylesheet}). If the theme didn't provide
    one, the default QRC stylesheet is copied with the configured filename.
*/
void TemplateGenerator::copyAssets()
{
    if (!m_context)
        return;

    const QString &outDir = m_context->outputDir.path();
    if (outDir.isEmpty())
        return;

    QSet<QString> themeAssets;

    if (!m_templateDir.isEmpty()) {
        QDir assetsDir(m_templateDir + "/assets"_L1);
        if (assetsDir.exists()) {
            QDirIterator it(assetsDir.path(), QDir::Files,
                            QDirIterator::Subdirectories);
            int count = 0;
            while (it.hasNext()) {
                it.next();
                QString rel = assetsDir.relativeFilePath(it.filePath());
                QString dst = outDir + '/'_L1 + rel;
                QDir().mkpath(QFileInfo(dst).path());
                QFile::remove(dst);
                if (QFile::copy(it.filePath(), dst)) {
                    themeAssets.insert(rel);
                    qCDebug(lcQDocTemplateGenerator) << "[%1]"_L1.arg(m_format) << "Asset (theme):" << rel;
                    ++count;
                } else {
                    qCWarning(lcQDocTemplateGenerator)
                            << "[%1]"_L1.arg(m_format) << "Failed to copy asset:" << it.filePath() << "->" << dst;
                }
            }
            if (count > 0)
                qCInfo(lcQDocTemplateGenerator) << "[%1]"_L1.arg(m_format) << "Copied" << count << "theme asset(s)";
        }
    }

    if (m_emitStylesheet && !themeAssets.contains(m_stylesheetName)) {
        const QString dstCss = outDir + '/'_L1 + m_stylesheetName;
        QFile::remove(dstCss);
        QFile::copy(":/qdoc/templates/assets/qdoc-default.css"_L1, dstCss);
        QFile(dstCss).setPermissions(QFile::ReadOwner | QFile::WriteOwner
                                     | QFile::ReadGroup | QFile::ReadOther);
        qCDebug(lcQDocTemplateGenerator) << "[%1]"_L1.arg(m_format) << "Asset (QRC fallback):" << m_stylesheetName;
    }
}

QT_END_NAMESPACE

