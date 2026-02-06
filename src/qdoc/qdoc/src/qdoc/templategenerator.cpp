// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "templategenerator.h"

#include "aggregate.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "documentationtraverser.h"
#include "filedocumentwriter.h"
#include "idocumentwriter.h"
#include "inclusionfilter.h"
#include "injabridge.h"
#include "ir/documentir.h"
#include "ir/irbuilder.h"
#include "namespacenode.h"
#include "node.h"
#include "outputcontext.h"
#include "outputproducerregistry.h"
#include "pagenode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"
#include "tree.h"
#include "utilities.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtextstream.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQDocTemplateGenerator, "qt.qdoc.templategenerator")

using namespace Qt::Literals;

/*!
    \class TemplateGenerator
    \internal
    \brief Generates documentation using external templates and a pre-built IR.

    TemplateGenerator implements IOutputProducer and IDocumentationHandler to
    generate documentation without inheriting from Generator. It uses
    DocumentationTraverser for tree traversal and delegates content generation
    to templates via the IR system.

    \section1 Architecture

    The generator follows a composition-based design:
    \list
        \li \b{IOutputProducer}: Lifecycle interface (prepare/produce/finalize).
        \li \b{IDocumentationHandler}: Content generation callbacks for
            traverser.
        \li \b{DocumentationTraverser}: Shared tree traversal logic.
        \li \b{IDocumentWriter}: Output abstraction (file or string for tests).
    \endlist

    \sa DocumentationTraverser, IDocumentationHandler, IOutputProducer,
        IRBuilder
*/

TemplateGenerator::TemplateGenerator(FileResolver &fileResolver, QDocDatabase &qdb)
    : m_fileResolver(fileResolver)
    , m_qdb(qdb)
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

    QString extensionConfig = config.get(u"template.extension"_s).asString();
    if (!extensionConfig.isEmpty())
        m_fileExtension = extensionConfig;

    QString templateDirConfig = config.get(u"template.templatedir"_s).asString();

    if (templateDirConfig.isEmpty()) {
        m_templateDir.clear();
    } else if (QDir::isAbsolutePath(templateDirConfig)) {
        m_templateDir = templateDirConfig;
    } else {
        // Relative path: resolve relative to output directory
        const QString &outDir = m_context->outputDir.path();
        if (!outDir.isEmpty())
            m_templateDir = outDir + "/"_L1 + templateDirConfig;
        else
            m_templateDir = templateDirConfig;
    }

    bool foundTemplates = false;
    if (!m_templateDir.isEmpty()) {
        QDir templateDir(m_templateDir);
        if (templateDir.exists() && !templateDir.entryList(QDir::Files).isEmpty()) {
            foundTemplates = true;
            qCInfo(lcQDocTemplateGenerator) << "Using template directory:" << m_templateDir;
        } else if (!templateDir.exists()) {
            qCInfo(lcQDocTemplateGenerator)
                    << "Configured template directory does not exist:" << m_templateDir
                    << "- will use embedded templates";
        } else {
            qCInfo(lcQDocTemplateGenerator)
                    << "Configured template directory is empty:" << m_templateDir
                    << "- will use embedded templates";
        }
    } else {
        qCInfo(lcQDocTemplateGenerator)
                << "No external template directory configured - will use embedded templates";
    }

    if (!foundTemplates)
        m_templateDir.clear();
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
    return "template"_L1;
}

void TemplateGenerator::beginDocument(const Node *node, const QString &outputFileName)
{
    Q_UNUSED(node);
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

    // Placeholder - IR integration pending
    if (m_writer && m_writer->isOpen()) {
        m_writer->writeLine(QString(u"<!-- TemplateGenerator: Collection "_s + cn->name() + u" -->"_s));
        m_writer->writeLine(QString(u"<h1>"_s + cn->fullTitle() + u"</h1>"_s));
        m_writer->writeLine(u"<p>Template-based output (IR integration pending)</p>"_s);
    }
}

void TemplateGenerator::generateGenericCollectionPage(CollectionNode *cn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    // Placeholder - IR integration pending
    if (m_writer && m_writer->isOpen()) {
        m_writer->writeLine(QString(u"<!-- TemplateGenerator: Generic Collection "_s + cn->name() + u" -->"_s));
        m_writer->writeLine(QString(u"<h1>"_s + cn->fullTitle() + u"</h1>"_s));
        m_writer->writeLine(u"<p>Template-based output (IR integration pending)</p>"_s);
    }
}

void TemplateGenerator::generatePageNode(PageNode *pn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    // Build phase: Node → IR (handled by IRBuilder)
    IRBuilder builder;
    DocumentIR ir = builder.buildPageIR(pn);

    // Render phase: IR → Output (TemplateGenerator's actual job)
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

    // Placeholder - IR integration pending
    if (m_writer && m_writer->isOpen()) {
        m_writer->writeLine(QString(u"<!-- TemplateGenerator: QML Type Page for "_s
                           + qcn->name() + u" -->"_s));
        m_writer->writeLine(QString(u"<h1>"_s + qcn->fullTitle() + u"</h1>"_s));
        m_writer->writeLine(u"<p>Template-based output (IR integration pending)</p>"_s);
    }
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

// === Public accessors ===

QString TemplateGenerator::fileExtension() const
{
    return m_fileExtension;
}

// === Private implementation ===

/*!
    \internal
    Render phase: Format pre-built IR according to a template.

    This is TemplateGenerator's core responsibility. It receives IR and
    produces formatted output without any knowledge of Nodes or the database.
*/
void TemplateGenerator::renderDocument(const DocumentIR &ir, const QString &templateBaseName)
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
        qFatal("TemplateGenerator: No template file found for extension '%s'. "
               "Ensure '%s.%s' exists in the configured template directory or in resources.",
               qPrintable(m_fileExtension), qPrintable(templateBaseName),
               qPrintable(m_fileExtension));

    QString rendered = InjaBridge::render(templateContent, ir.toJson());

    if (m_writer && m_writer->isOpen())
        m_writer->write(rendered);
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

/*!
    \internal
    Computes the base filename for a node, adapted from Generator::fileBase().

    This handles the various node types (collections, pages, QML types, etc.)
    and applies the configured prefixes and suffixes.
*/
QString TemplateGenerator::fileBase(const Node *node) const
{
    if (!node->isPageNode() && !node->isCollectionNode())
        node = node->parent();

    if (node->hasFileNameBase())
        return node->fileNameBase();

    QString base{node->name()};
    if (base.endsWith(".html"_L1))
        base.truncate(base.size() - 5);

    const QString typeKey = nodeTypeKey(node);

    if (node->isCollectionNode()) {
        if (node->isQmlModule())
            base.append("-qmlmodule"_L1);
        else if (node->isModule())
            base.append("-module"_L1);
        base.append(m_context->outputSuffix(typeKey));
    } else if (node->isTextPageNode()) {
        if (node->isExample()) {
            base.prepend(u"%1-"_s.arg(m_context->project.toLower()));
            base.append("-example"_L1);
        }
    } else if (node->isQmlType()) {
        /*
          To avoid file name conflicts in the html directory,
          we prepend a prefix (by default, "qml-") and an optional suffix
          to the file name. The suffix, if one exists, is appended to the
          module name.

          For historical reasons, skip the module name qualifier for QML value types
          in order to avoid excess redirects in the online docs.
        */
        if (!node->logicalModuleName().isEmpty() && !node->isQmlBasicType()) {
            const InclusionPolicy policy = Config::instance().createInclusionPolicy();
            const NodeContext context = node->logicalModule()->createContext();
            if (InclusionFilter::isIncluded(policy, context))
                base.prepend(u"%1%2-"_s.arg(node->logicalModuleName(), m_context->outputSuffix(typeKey)));
        }
    } else if (node->isProxyNode()) {
        base.append(u"-%1-proxy"_s.arg(node->tree()->physicalModuleName()));
    } else {
        base.clear();
        const Node *p = node;
        forever {
            const Node *pp = p->parent();
            base.prepend(p->name());
            if (pp == nullptr || pp->name().isEmpty() || pp->isTextPageNode())
                break;
            base.prepend('-'_L1);
            p = pp;
        }
        if (node->isNamespace() && !node->name().isEmpty()) {
            const auto *ns = static_cast<const NamespaceNode *>(node);
            if (!ns->isDocumentedHere()) {
                base.append("-sub-"_L1);
                base.append(ns->tree()->camelCaseModuleName());
            }
        }
        base.append(m_context->outputSuffix(typeKey));
    }

    base.prepend(m_context->outputPrefix(typeKey));
    QString canonicalName{Utilities::asAsciiPrintable(base)};

    // Cache the computed filename on the node for future lookups
    // (const_cast is safe here as we're just caching)
    auto *mutableNode = const_cast<Node *>(node);
    mutableNode->setFileNameBase(canonicalName);

    return canonicalName;
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

QT_END_NAMESPACE
