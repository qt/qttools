// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "builder.h"

#include "pagemetadata.h"

#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

namespace IR {

static void gatherBodyTocEntries(
        const QList<ContentBlock> &blocks,
        QList<NavigationInfo::TocEntry> &out)
{
    for (const auto &block : blocks) {
        if (block.type == BlockType::SectionHeading) {
            if (!block.attributes.contains("sectionRef"_L1))
                continue;
            const QString anchorId = block.attributes.value("sectionRef"_L1).toString();
            if (anchorId.isEmpty())
                continue;
            const int level = block.attributes.value("level"_L1).toInt(3);
            out.append({block.plainText(), anchorId, level});
        } else if (block.type == BlockType::Section) {
            gatherBodyTocEntries(block.children, out);
        }
    }
}

static InlineContent makeTextInline(const QString &text)
{
    InlineContent ic;
    ic.type = InlineType::Text;
    ic.text = text;
    return ic;
}

static InlineContent makeTopicLink(const QString &topicName)
{
    InlineContent ic;
    ic.type = InlineType::Link;
    ic.href = topicName;
    ic.link = InlineContent::LinkData{LinkOrigin::Explicit, LinkState::Unresolved};
    ic.children.append(makeTextInline(topicName));
    return ic;
}

static InlineContent makeResolvedLink(const QString &name, const QString &href)
{
    InlineContent ic;
    ic.type = InlineType::Link;
    ic.href = href;
    ic.link = InlineContent::LinkData{LinkOrigin::Explicit, LinkState::Resolved};
    ic.children.append(makeTextInline(name));
    return ic;
}

static void appendExceptionList(
        QList<InlineContent> &inlines,
        const QString &prefix,
        const QString &topicName,
        const QList<CppReferenceData::ThreadSafetyExceptionEntry> &exceptions)
{
    if (exceptions.isEmpty())
        return;
    inlines.append(makeTextInline(prefix));
    inlines.append(makeTopicLink(topicName));
    inlines.append(makeTextInline(": "_L1));
    for (qsizetype i = 0; i < exceptions.size(); ++i) {
        if (i > 0)
            inlines.append(makeTextInline(", "_L1));
        const auto &exc = exceptions[i];
        if (!exc.href.isEmpty())
            inlines.append(makeResolvedLink(exc.name, exc.href));
        else
            inlines.append(makeTextInline(exc.name));
    }
    inlines.append(makeTextInline("."_L1));
}

static QList<ContentBlock> buildThreadSafetyAdmonition(
        const CppReferenceData::ThreadSafetyInfo &ts,
        const QString &typeWord)
{
    QList<InlineContent> inlines;
    const bool hasExceptions = !ts.reentrantExceptions.isEmpty()
            || ts.threadSafeExceptions.isEmpty() == false
            || !ts.nonReentrantExceptions.isEmpty();

    if (ts.level == "non-reentrant"_L1) {
        inlines.append(makeTextInline("This "_L1 + typeWord + " is not "_L1));
        inlines.append(makeTopicLink("reentrant"_L1));
        inlines.append(makeTextInline("."_L1));
        appendExceptionList(inlines, " These functions are "_L1, "reentrant"_L1,
                            ts.reentrantExceptions);
    } else if (ts.level == "reentrant"_L1) {
        inlines.append(makeTextInline("All functions in this "_L1 + typeWord + " are "_L1));
        inlines.append(makeTopicLink("reentrant"_L1));
        const bool hasThreadSafe = !ts.threadSafeExceptions.isEmpty();
        if (hasExceptions && !hasThreadSafe)
            inlines.append(makeTextInline(" with the following exceptions:"_L1));
        else
            inlines.append(makeTextInline("."_L1));
        appendExceptionList(inlines, " These functions are not "_L1, "reentrant"_L1,
                            ts.nonReentrantExceptions);
        appendExceptionList(inlines, " These functions are also "_L1, "thread-safe"_L1,
                            ts.threadSafeExceptions);
    } else if (ts.level == "thread-safe"_L1) {
        inlines.append(makeTextInline("All functions in this "_L1 + typeWord + " are "_L1));
        inlines.append(makeTopicLink("thread-safe"_L1));
        if (hasExceptions)
            inlines.append(makeTextInline(" with the following exceptions:"_L1));
        else
            inlines.append(makeTextInline("."_L1));
        appendExceptionList(inlines, " These functions are only "_L1, "reentrant"_L1,
                            ts.reentrantExceptions);
        appendExceptionList(inlines, " These functions are not "_L1, "reentrant"_L1,
                            ts.nonReentrantExceptions);
    }

    ContentBlock block;
    block.type = BlockType::Paragraph;
    block.inlineContent = std::move(inlines);
    return {block};
}

/*!
    \class IR::Builder
    \internal
    \brief Assembles IR Documents from pre-extracted metadata.

    Builder consumes PageMetadata, a value-type struct populated by the
    driver-side extraction layer (NodeExtractor). It copies pre-extracted
    fields into an IR::Document without touching Node subclass headers,
    Atom chains, or the documentation database.

    This separation means Builder has no dependencies on the legacy Node
    layer and is eligible for QDocLib migration. Generators receive
    pre-built IR and focus purely on formatting output.

    \section1 Content Pipeline

    Content arrives pre-built as a list of ContentBlock values in
    PageMetadata::body. ContentBuilder (called at extraction time)
    handles the atom-to-block transformation, including brief
    exclusion. Format-conditional atoms are skipped unconditionally
    since the IR is format-agnostic. Builder's role is assembly, not
    transformation.

    \section1 Flat Text Fallback

    Builder computes a flat text representation from the structured body
    for \c{content.text}. This is transitional — templates will consume
    \c{content.blocks} directly once content rendering is in place.

    \sa IR::Document, IR::PageMetadata, TemplateGenerator
*/


/*!
    \internal
    Assemble an IR Document from pre-extracted PageMetadata.

    Classification, identity, and content fields are moved from
    \a pm. The body (a list of ContentBlock values built by
    ContentBuilder at extraction time) is transferred as-is. A flat
    text fallback is computed until templates consume the structured
    body directly.
*/
Document Builder::buildPageIR(PageMetadata pm) const
{
    Document ir;

    ir.nodeType = pm.nodeType;
    ir.genus = pm.genus;
    ir.status = pm.status;
    ir.access = pm.access;

    ir.title = std::move(pm.title);
    ir.fullTitle = std::move(pm.fullTitle);
    ir.url = std::move(pm.url);
    ir.since = std::move(pm.since);
    ir.deprecatedSince = std::move(pm.deprecatedSince);
    ir.brief = std::move(pm.brief);

    ir.body = std::move(pm.body);
    ir.summarySections = std::move(pm.summarySections);
    ir.detailSections = std::move(pm.detailSections);

    if (pm.qmlTypeData) {
        const auto &src = *pm.qmlTypeData;
        QmlTypeInfo info;
        info.importStatement = src.importStatement;
        info.isSingleton = src.isSingleton;
        info.isValueType = src.isValueType;

        if (src.inherits) {
            info.inherits = QmlTypeInfo::InheritsInfo{
                src.inherits->name, src.inherits->href, src.inherits->moduleName
            };
        }

        for (const auto &entry : src.inheritedBy)
            info.inheritedBy.append({entry.name, entry.href});

        if (src.nativeType)
            info.nativeType = QmlTypeInfo::NativeTypeInfo{src.nativeType->name, src.nativeType->href};

        ir.qmlTypeInfo = std::move(info);
    }

    if (pm.collectionData) {
        const auto &src = *pm.collectionData;
        CollectionInfo info;
        info.logicalModuleName = src.logicalModuleName;
        info.logicalModuleVersion = src.logicalModuleVersion;
        info.qtVariable = src.qtVariable;
        info.cmakePackage = src.cmakePackage;
        info.cmakeComponent = src.cmakeComponent;
        info.cmakeTargetItem = src.cmakeTargetItem;
        info.state = src.state;

        info.isModule = src.isModule;
        info.isQmlModule = src.isQmlModule;
        info.isGroup = src.isGroup;
        info.noAutoList = src.noAutoList;

        for (const auto &entry : src.namespaces)
            info.namespaces.append({entry.name, entry.href, entry.brief});
        for (const auto &entry : src.classes)
            info.classes.append({entry.name, entry.href, entry.brief});
        for (const auto &entry : src.members)
            info.members.append({entry.name, entry.href, entry.brief});

        ir.collectionInfo = std::move(info);
    }

    if (pm.cppReferenceData) {
        const auto &src = *pm.cppReferenceData;
        CppReferenceInfo info;

        info.headerInclude = src.headerInclude;
        info.cmakeFindPackage = src.cmakeFindPackage;
        info.cmakeTargetLinkLibraries = src.cmakeTargetLinkLibraries;
        info.qmakeVariable = src.qmakeVariable;
        info.statusText = src.statusText;
        info.statusCssClass = src.statusCssClass;

        if (src.qmlNativeType)
            info.qmlNativeType = CppReferenceInfo::QmlNativeTypeLink{
                src.qmlNativeType->name, src.qmlNativeType->href};

        for (const auto &bc : src.baseClasses)
            info.baseClasses.append({bc.name, bc.href, bc.access});
        for (const auto &dc : src.derivedClasses)
            info.derivedClasses.append({dc.name, dc.href});
        info.suppressInheritance = src.suppressInheritance;

        info.templateDeclSpans = src.templateDeclSpans;

        info.isInnerClass = src.isInnerClass;
        info.isNamespace = src.isNamespace;
        info.isHeader = src.isHeader;

        info.isPartialNamespace = src.isPartialNamespace;
        info.fullNamespaceHref = src.fullNamespaceHref;
        info.fullNamespaceModuleName = src.fullNamespaceModuleName;

        info.typeWord = src.typeWord;
        info.ancestorNames = src.ancestorNames;

        info.selfComparisonCategory = src.selfComparisonCategory;
        for (const auto &ce : src.comparisonEntries)
            info.comparisonEntries.append({ce.category, ce.comparableTypes, ce.description});

        if (src.threadSafety) {
            CppReferenceInfo::ThreadSafetyInfo ts;
            ts.level = src.threadSafety->level;
            for (const auto &e : src.threadSafety->reentrantExceptions)
                ts.reentrantExceptions.append({e.name, e.href});
            for (const auto &e : src.threadSafety->threadSafeExceptions)
                ts.threadSafeExceptions.append({e.name, e.href});
            for (const auto &e : src.threadSafety->nonReentrantExceptions)
                ts.nonReentrantExceptions.append({e.name, e.href});
            info.threadSafety = std::move(ts);
            info.threadSafetyAdmonition = buildThreadSafetyAdmonition(
                    *src.threadSafety, src.typeWord);
        }

        for (const auto &g : src.groups)
            info.groups.append({g.name, g.href});

        info.hasObsoleteMembers = src.hasObsoleteMembers;
        // TODO: obsoleteMembersUrl is currently set by the generator
        // after assembly because it depends on the file extension.
        // This post-build mutation violates frozen-IR. The Builder
        // should receive the file extension and compute it here.

        ir.cppReferenceInfo = std::move(info);
    }

    {
        const auto &src = pm.navigationData;
        NavigationInfo info;
        for (const auto &bc : src.breadcrumbs) {
            NavigationInfo::BreadcrumbEntry entry;
            entry.title = bc.title;
            entry.href = bc.href;
            switch (bc.state) {
            case NavigationData::CrumbState::Link:
                entry.state = NavigationInfo::CrumbState::Link;
                break;
            case NavigationData::CrumbState::Current:
                entry.state = NavigationInfo::CrumbState::Current;
                break;
            case NavigationData::CrumbState::Unresolved:
                entry.state = NavigationInfo::CrumbState::Unresolved;
                break;
            }
            info.breadcrumbs.append(std::move(entry));
        }
        if (src.previousLink)
            info.previousLink = NavigationInfo::LinkEntry{
                    src.previousLink->title, src.previousLink->href};
        if (src.nextLink)
            info.nextLink = NavigationInfo::LinkEntry{
                    src.nextLink->title, src.nextLink->href};
        if (src.startLink)
            info.startLink = NavigationInfo::LinkEntry{
                    src.startLink->title, src.startLink->href};
        info.tocDepth = src.tocDepth;

        // Assemble TOC entries from page structure (summary sections +
        // "Detailed Description" divider + body section-headings +
        // detail sections). Order reflects the rendered page, so templates
        // can iterate for a top-to-bottom reading of available anchors.
        for (const auto &s : ir.summarySections) {
            if (!s.title.isEmpty() && !s.id.isEmpty())
                info.tocEntries.append({s.title, s.id, 2});
        }
        if (ir.cppReferenceInfo && !ir.body.isEmpty())
            info.tocEntries.append(
                    {u"Detailed Description"_s, u"details"_s, 2});
        gatherBodyTocEntries(ir.body, info.tocEntries);
        for (const auto &s : ir.detailSections) {
            if (!s.title.isEmpty() && !s.id.isEmpty())
                info.tocEntries.append({s.title, s.id, 2});
        }

        const bool hasAnyNavigation = !info.breadcrumbs.isEmpty()
                || info.previousLink || info.nextLink || info.startLink
                || !info.tocEntries.isEmpty() || info.tocDepth != -1;
        if (hasAnyNavigation)
            ir.navigationInfo = std::move(info);
    }

    // Transitional: templates don't yet consume content.blocks.
    QStringList paragraphs;
    for (const auto &block : ir.body) {
        const QString text = block.plainText();
        if (!text.isEmpty())
            paragraphs.append(text);
    }
    ir.contentJson["text"_L1] = paragraphs.join("\n\n"_L1);

    return ir;
}

} // namespace IR

QT_END_NAMESPACE
