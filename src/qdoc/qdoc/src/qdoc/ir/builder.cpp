// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "builder.h"

#include "pagemetadata.h"

#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

namespace IR {

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
