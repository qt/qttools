// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_CONTENTBUILDER_H
#define QDOC_IR_CONTENTBUILDER_H

#include "contentblock.h"

#include <QJsonObject>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE

class Atom;

namespace IR {

class ContentBuilder
{
public:
    explicit ContentBuilder(const QString &format = {});
    QList<ContentBlock> build(const Atom *firstAtom);

private:
    void processAtoms(const Atom *atom);
    const Atom *processUntilBoundary(const Atom *atom);
    const Atom *processFormatIf(const Atom *atom);
    const Atom *skipUntilBoundary(const Atom *atom);
    const Atom *skipFormatIf(const Atom *atom);
    const Atom *dispatchAtom(const Atom *atom);

    void openBlock(BlockType type, QJsonObject attrs = {});
    void closeBlock();
    void addInline(InlineContent inline_);
    void addLeafInline(InlineType type, const QString &text);
    void pushInlineContainer(InlineContent container);

    ContentBlock *resolveBlock();
    InlineContent *resolveInline();

    QString m_format;
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

    bool m_inBrief = false;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_CONTENTBUILDER_H
