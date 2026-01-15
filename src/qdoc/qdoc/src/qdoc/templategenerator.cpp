// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "templategenerator.h"

#include "aggregate.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "config.h"
#include "node.h"
#include "pagenode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"

#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

QT_BEGIN_NAMESPACE

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
}

void TemplateGenerator::terminateGenerator()
{
    Generator::terminateGenerator();
}

QString TemplateGenerator::format()
{
    return "template"_L1;
}

[[nodiscard]] QString TemplateGenerator::fileExtension() const
{
    // TODO: Make this configurable via template configuration
    return "html"_L1;
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

    out() << "<!-- TemplateGenerator: Page for "
          << pn->name() << " -->\n";
    out() << "<h1>" << pn->fullTitle() << "</h1>\n";
    out() << "<p>Template-based output (IR integration pending)</p>\n";
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

