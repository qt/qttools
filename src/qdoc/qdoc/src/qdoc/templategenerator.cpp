// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "templategenerator.h"

#include "aggregate.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "filedocumentwriter.h"
#include "idocumentwriter.h"
#include "injabridge.h"
#include "ir/documentir.h"
#include "ir/irbuilder.h"
#include "node.h"
#include "outputcontext.h"
#include "pagenode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtextstream.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQDocTemplateGenerator, "qt.qdoc.templategenerator")

using namespace Qt::Literals;

/*!
    \class TemplateGenerator
    \internal
    \brief Generates documentation using external templates and a pre-built IR.

    TemplateGenerator is designed to consume a complete IR (Intermediate
    Representation) that contains all resolved links, organized content
    sections, and file paths. The generator itself is "dumb": it only
    formats pre-resolved data according to template rules, without
    performing resolution, database lookups, or state modifications.

    \section1 Architecture

    The generator follows QDoc's compile/link/render pipeline:

    \list
    \li \b{Build phase} (IRBuilder): Extract data from Node tree into IR.
        Handles all atom processing and Node interaction.
    \li \b{Link phase} (HrefResolver, future): Resolve cross-module links.
    \li \b{Render phase} (TemplateGenerator): Format IR into output.
        Implemented by \c{renderDocument()}, which knows nothing about Nodes.
    \endlist

    The \c{generateXxx()} methods inherited from Generator are thin wrappers
    that call IRBuilder to build IR, then renderDocument() to format it.
    This separation ensures the render phase can be tested independently
    and that IR design is driven by actual rendering needs.

    \sa IRBuilder, DocumentIR
*/

TemplateGenerator::TemplateGenerator(FileResolver& file_resolver)
    : Generator(file_resolver)
{
}

TemplateGenerator::~TemplateGenerator() = default;

void TemplateGenerator::initializeGenerator()
{
    Generator::initializeGenerator();

    // Initialize output context and writer first, so we can use m_context
    // for output directory resolution below
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

void TemplateGenerator::terminateGenerator()
{
    m_writer.reset();
    Generator::terminateGenerator();
}

/*!
    \internal
    Creates the production FileDocumentWriter.

    This is called during initialization to create the default writer
    that writes to the filesystem. For testing, a mock writer can be
    injected by setting m_writer before calling initializeGenerator().

    Also initializes m_context with configuration values, enabling
    OutputContext-based access to output settings without depending
    on Generator's static variables.
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

QString TemplateGenerator::format() const
{
    return "template"_L1;
}

[[nodiscard]] QString TemplateGenerator::fileExtension() const
{
    return m_fileExtension;
}

void TemplateGenerator::generateDocs()
{
    // TODO: This will be replaced with IR-based generation
    // For now, call the base implementation to demonstrate integration
    Generator::generateDocs();
}

/*
    Placeholder implementation.

    \note File management is handled by Generator::generateDocumentation().
          This method only writes content.
 */
void TemplateGenerator::generateCppReferencePage(Aggregate *aggregate, CodeMarker *marker)
{
    Q_UNUSED(marker);

    // TODO: Load template, populate with IR data, render
    out() << "<!-- TemplateGenerator: C++ Reference Page for "
          << aggregate->name() << " -->\n";
    out() << "<h1>" << aggregate->fullTitle() << "</h1>\n";
    out() << "<p>Template-based output (IR integration pending)</p>\n";
}

void TemplateGenerator::generateQmlTypePage(QmlTypeNode *qcn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    // TODO: Load template, populate with IR data, render
    out() << "<!-- TemplateGenerator: QML Type Page for "
          << qcn->name() << " -->\n";
    out() << "<h1>" << qcn->fullTitle() << "</h1>\n";
    out() << "<p>Template-based output (IR integration pending)</p>\n";
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

    // Use IDocumentWriter if available and open (e.g., in tests), otherwise
    // fall back to Generator's out() stream for production compatibility.
    if (m_writer && m_writer->isOpen())
        m_writer->write(rendered);
    else
        out() << rendered;
}

void TemplateGenerator::generateCollectionNode(CollectionNode *cn, CodeMarker *marker)
{
    Q_UNUSED(marker);

    out() << "<!-- TemplateGenerator: Collection "
          << cn->name() << " -->\n";
    out() << "<h1>" << cn->fullTitle() << "</h1>\n";
    out() << "<p>Template-based output (IR integration pending)</p>\n";
}

/*!
    \internal
    Stub implementation - not yet IR-driven.

    This method emits placeholder HTML comments and should not be relied upon
    for actual content rendering. Atom processing will be integrated with the
    IR system in future commits.
*/
qsizetype TemplateGenerator::generateAtom(const Atom *atom, const Node *relative,
                                          CodeMarker *marker)
{
    Q_UNUSED(relative);
    Q_UNUSED(marker);

    // TODO: This will be replaced with template-based rendering
    if (atom) {
        out() << "<!-- Atom: " << atom->typeString() << " -->\n";
        return 1;
    }
    return 0;
}

QT_END_NAMESPACE

