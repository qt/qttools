// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "nodeextractor.h"

#include "atom.h"
#include "doc.h"
#include "ir/contentbuilder.h"
#include "pagenode.h"
#include "text.h"

QT_BEGIN_NAMESPACE

namespace NodeExtractor {

/*!
    \internal
    Extract page-level metadata from a PageNode into a value-type struct.

    This function reads classification, identity, brief, and body fields
    from the given PageNode and returns them as an IR::PageMetadata value.
    Body content is populated via ContentBuilder, which transforms the
    atom chain into structured content blocks.

    The \a format parameter is passed to ContentBuilder for FormatIf
    evaluation in format-conditional documentation.

    The caller (TemplateGenerator) invokes this before passing the
    result to IR::Builder, ensuring Builder never includes PageNode
    or other Node subclass headers.
*/
IR::PageMetadata extractPageMetadata(const PageNode *pn, const QString &format)
{
    Q_ASSERT_X(pn, "NodeExtractor::extractPageMetadata",
               "PageNode pointer must be non-null");
    IR::PageMetadata pm;

    pm.nodeType = pn->nodeType();
    pm.genus = pn->genus();
    pm.status = pn->status();
    pm.access = pn->access();

    pm.title = pn->title();
    pm.fullTitle = pn->fullTitle();
    pm.url = pn->url();
    pm.since = pn->since();
    pm.deprecatedSince = pn->deprecatedSince();
    pm.brief = pn->doc().briefText().toString();

    const Text &bodyText = pn->doc().body();
    if (const Atom *firstAtom = bodyText.firstAtom()) {
        IR::ContentBuilder contentBuilder(format);
        pm.body = contentBuilder.build(firstAtom);
    }

    return pm;
}

} // namespace NodeExtractor

QT_END_NAMESPACE
