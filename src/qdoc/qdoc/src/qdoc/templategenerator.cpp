// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "templategenerator.h"

#include "aggregate.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "injabridge.h"
#include "ir/documentir.h"
#include "node.h"
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

    The generator maintains a strict separation between two phases:

    \list
    \li \b{Build phase}: Extract data from Node tree into IR structures.
        Currently implemented as \c{buildXxxIR()} methods, but will move
        to a separate IRBuilder class.
    \li \b{Render phase}: Format IR into output using templates.
        Implemented by \c{renderDocument()}, which knows nothing about Nodes.
    \endlist

    The \c{generateXxx()} methods inherited from Generator are thin wrappers
    that call build then render. This separation ensures the render phase
    can be tested independently and that IR design is driven by actual
    rendering needs.

    This serves as both a working generator and a reference implementation
    that drives the design of QDoc's IR layer.
*/

TemplateGenerator::TemplateGenerator(FileResolver& file_resolver)
    : Generator(file_resolver)
{
}

void TemplateGenerator::initializeGenerator()
{
    Generator::initializeGenerator();

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
        QString outDir = outputDir();
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
    Generator::terminateGenerator();
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

    // Build phase: Node → IR (will move to IRBuilder)
    DocumentIR ir = buildPageIR(pn);

    // Render phase: IR → Output (TemplateGenerator's actual job)
    renderDocument(ir, "page"_L1);
}

/*!
    \internal
    Build phase: Extract documentation data from a PageNode into IR.

    This method will eventually move to a separate IRBuilder class.
    The TemplateGenerator should receive pre-built IR, not build it.
*/
DocumentIR TemplateGenerator::buildPageIR(const PageNode *pn) const
{
    DocumentIR ir;
    ir.title = pn->title();
    ir.fullTitle = pn->fullTitle();
    ir.url = pn->url();
    ir.brief = pn->doc().briefText().toString();

    // TODO: Process atoms in later commits
    ir.contentJson["text"_L1] = "Content rendering from atoms will be implemented in upcoming commits."_L1;

    return ir;
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

