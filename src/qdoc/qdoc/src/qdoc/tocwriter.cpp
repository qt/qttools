// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tocwriter.h"

#include "generator.h"
#include "location.h"
#include "qdocdatabase.h"
#include "node.h"

#include <QtCore/qfile.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class TOCWriter
    \brief Table of contents writer.

    Generates an XML file for the table of contents (TOC) for the documentation
    project. The TOC consists of (a nested) list of sub-pages
    documented with \\toc command(s).
*/


/*!
    Constructs a new TOCWriter, using a Generator \a g to
    to fetch links (hrefs) for each TOC entry. A \a project
    name is used in error reporting.
*/
TOCWriter::TOCWriter(Generator *g, const QString &project)
: m_gen(g),
  m_qdb(QDocDatabase::qdocDB()),
  m_project(project)
{
}

/*!
    Writes the TOC entries for \a project to \a fileName,
    starting from a page with a title matching \a indexTitle.

    Does nothing if \a indexTitle is an empty string.
*/
void TOCWriter::generateTOC(const QString &fileName, const QString &indexTitle)
{
    if (indexTitle.isEmpty())
        return;

    auto *root = m_qdb->findNodeForTarget(indexTitle, nullptr);
    if (!root)
        return;

    // Do not write an empty file
    if (!getEntries(root))
        return;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        Location().error("Failed to open %1 for writing"_L1.arg(fileName));
        return;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(u"section"_s);
    writer.writeAttribute(u"ref"_s, m_gen->fullDocumentLocation(root));
    writer.writeAttribute(u"title"_s, indexTitle);
    writeEntry(writer, root);
    writer.writeEndElement(); // root section
    writer.writeEndDocument();
    file.close();
}


/*!
    Recursively writes the TOC entries (`section` elements) for \a node,
    using \a writer.
*/
void TOCWriter::writeEntry(QXmlStreamWriter &writer, const Node *node)
{
    const auto entries{getEntries(node)};
    if (!entries)
        return;

    // Max allowed recursion depth
    constexpr int maxDepth{16};

    if (++m_recursionDepth > maxDepth) {
        Location().error("Maximum nesting level (%1) exceeded "
                         "when writing table of contents for '%2'"_L1
                            .arg(QString::number(maxDepth), m_project));
    } else {
        for (const auto &e : *entries) {
            writer.writeStartElement(u"section"_s);
            writer.writeAttribute(u"ref"_s, m_gen->fullDocumentLocation(e.first));
            writer.writeAttribute(u"title"_s, e.second);
            writeEntry(writer, e.first);
            writer.writeEndElement(); // section
        }
    }
    --m_recursionDepth;
}

/*!
    Returns a list of TOC entries for \a node. A single TOC entry is
    a pair consisting of a Node pointer and a user-visible link text.
*/
std::optional<TOCWriter::TitledNodeList> TOCWriter::getEntries(const Node *node) const
{
    const auto *atom = node->doc().body().firstAtom();
    if (atom)
        atom = atom->find(Atom::TableOfContentsLeft);
    if (!atom)
        return std::nullopt;

    TitledNodeList result;
    while (atom->next() && !atom->next(Atom::TableOfContentsRight)) {
        atom = atom->next();
        if (atom->type() == Atom::Link) {
            QString ref{};
            if (const auto *target = m_qdb->findNodeForAtom(atom, nullptr, ref))
                result.append(std::make_pair(target, atom->linkText()));
            else if (!Generator::noLinkErrors())
                node->doc().location().warning("Can't link to '%1'"_L1.arg(atom->string()));
        }
    }

    if (result.isEmpty())
        return std::nullopt;
    return result;
}

QT_END_NAMESPACE
