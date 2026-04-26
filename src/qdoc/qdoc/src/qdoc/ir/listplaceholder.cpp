// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "listplaceholder.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

namespace IR {

/*!
    \enum IR::ListPlaceholderVariant
    \internal
    \brief Discriminator naming the catalog shape carried by a
    \c{BlockType::ListPlaceholder} block.

    The content builder picks a variant when emitting a list-placeholder
    for \\generatelist or \\annotatedlist; the list-expander pass
    matches on the variant when populating the catalog. Adding a
    catalog form means adding an enumerator here and a matching arm in
    \c{toString()}, so a missing string mapping fails to compile.

    \value AnnotatedGroup The annotated members of a documentation
        group, emitted for \\annotatedlist \e{<group>}.
    \value AnnotatedExamples The annotated list of example projects,
        emitted for \\generatelist annotatedexamples.
    \value AnnotatedClasses The annotated list of public classes,
        emitted for \\generatelist annotatedclasses.
    \value CompactClasses The compact, alphabetically-grouped classes
        index, emitted for \\generatelist classes \e{[<rootname>]}.
*/

/*!
    \fn QString IR::toString(ListPlaceholderVariant variant)
    \internal

    Returns the canonical kebab-case string used as the \c variant
    attribute of the placeholder block in JSON output. The string is
    the contract between the content builder, the list-expander pass,
    and the templates: every consumer matches on the same value, and
    \c{toString()} is the single source of truth.
*/
QString toString(ListPlaceholderVariant variant)
{
    switch (variant) {
    case ListPlaceholderVariant::AnnotatedGroup:    return u"annotated-group"_s;
    case ListPlaceholderVariant::AnnotatedExamples: return u"annotated-examples"_s;
    case ListPlaceholderVariant::AnnotatedClasses:  return u"annotated-classes"_s;
    case ListPlaceholderVariant::CompactClasses:    return u"compact-classes"_s;
    }
    Q_UNREACHABLE_RETURN({});
}

} // namespace IR

QT_END_NAMESPACE
