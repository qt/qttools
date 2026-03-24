// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_CONTENTBUILDER_H
#define QDOC_IR_CONTENTBUILDER_H

#include "contentblock.h"

#include <QJsonObject>
#include <QList>
#include <QString>

#include <functional>

QT_BEGIN_NAMESPACE

class Atom;

namespace IR {

using DiagnosticHandler = std::function<void(QtMsgType, const QString &)>;

/*!
    \enum IR::BriefHandling
    \internal

    Controls whether ContentBuilder emits brief content into the body.

    \value Skip Brief content between BriefLeft/BriefRight is suppressed.
        This is the default for IR::Builder, where the brief is stored
        as a separate field in IR::Document.
    \value Include Brief content is emitted as a normal Paragraph block.
        This enables callers that need the brief rendered as part of
        the body, matching the behavior where the brief appears as the
        opening paragraph.
*/
enum class BriefHandling { Skip, Include };

class ContentBuilder
{
public:
    explicit ContentBuilder(BriefHandling briefHandling = BriefHandling::Skip,
                            int headingOffset = 0,
                            DiagnosticHandler diagnosticHandler = {});
    QList<ContentBlock> build(const Atom *firstAtom);

private:
    void processAtoms(const Atom *atom);
    const Atom *skipFormatIfBlock(const Atom *atom);
    const Atom *dispatchAtom(const Atom *atom);

    void openBlock(BlockType type, QJsonObject attrs = {});
    void closeBlock();
    void addInline(InlineContent inline_);
    void addLeafInline(InlineType type, const QString &text);
    void pushInlineContainer(InlineContent container);

    ContentBlock *resolveBlock();
    InlineContent *resolveInline();

    QList<ContentBlock> m_result;

    // Index path through m_result / children hierarchy.
    // First index selects from m_result; subsequent indices descend
    // through children lists. Indices remain valid as long as we only
    // append and never remove or reorder items.
    QList<qsizetype> m_blockPath;

    QList<qsizetype> m_inlinePath;

    // Records m_inlinePath.size() at each openBlock() call, so that
    // closeBlock() can verify/restore inline depth (one entry per
    // m_blockPath entry, always in sync).
    QList<qsizetype> m_inlineBaseDepths;

    BriefHandling m_briefHandling = BriefHandling::Skip;
    int m_headingOffset = 0;
    bool m_inBrief = false;
    bool m_inLink = false;
    DiagnosticHandler m_diagnose;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_CONTENTBUILDER_H
