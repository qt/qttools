// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "irbuilder.h"

#include "../atom.h"
#include "../pagenode.h"
#include "../text.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

/*!
    \class IRBuilder
    \internal
    \brief Builds IR (Intermediate Representation) from QDoc's Node tree.

    IRBuilder is the "compiler" in QDoc's compile/link/render pipeline. It
    transforms Node objects into format-agnostic IR structures that can be:

    \list
    \li Rendered to output formats (HTML, Markdown, DocBook) by generators.
    \li Written to .index files for cross-module linking.
    \li Consumed by other IR processors.
    \endlist

    \section1 Separation of Concerns

    IRBuilder handles all interaction with Node classes and Atom chains.
    Generators receive pre-built IR and focus purely on formatting output.
    This separation enables:

    \list
    \li Testing IR building independently from rendering.
    \li Multiple output formats from the same IR.
    \li Clear architectural boundaries.
    \endlist

    \section1 Link Resolution

    During IR building, \b{local links} (within the same module) are resolved
    immediately. \b{Cross-module links} are marked as external with an empty
    href, to be resolved during the link phase when dependency .index files
    are available.

    \sa DocumentIR, TemplateGenerator
*/


/*!
    \internal
    Build IR for a PageNode.

    This method extracts documentation content from the node's atom chain.
    The brief is stored separately via Doc::briefText(), while body content
    is extracted by walking the atom chain and collecting text atoms that
    are not within the brief section.

    \note Currently handles basic text atoms (String, AutoLink, C) and
    paragraph breaks. More complex atom types (lists, code blocks, images)
    will be added as the IR layer matures.
*/
DocumentIR IRBuilder::buildPageIR(const PageNode *pn) const
{
    DocumentIR ir;

    // Classification
    ir.nodeType = pn->nodeType();
    ir.genus = pn->genus();
    ir.status = pn->status();
    ir.access = pn->access();

    // Identity
    ir.title = pn->title();
    ir.fullTitle = pn->fullTitle();
    ir.url = pn->url();
    ir.since = pn->since();
    ir.brief = pn->doc().briefText().toString();

    QString bodyText;
    const Text &body = pn->doc().body();
    const Atom *atom = body.firstAtom();
    bool inBrief = false;

    while (atom) {
        switch (atom->type()) {
        case Atom::BriefLeft:
            inBrief = true;
            break;
        case Atom::BriefRight:
            inBrief = false;
            break;
        case Atom::ParaLeft:
            if (!inBrief && !bodyText.isEmpty())
                bodyText += "\n\n"_L1;
            break;
        case Atom::String:
        case Atom::AutoLink:
        case Atom::C:
            if (!inBrief)
                bodyText += atom->string();
            break;
        default:
            break;
        }
        atom = atom->next();
    }

    ir.contentJson["text"_L1] = bodyText.trimmed();

    return ir;
}

QT_END_NAMESPACE

