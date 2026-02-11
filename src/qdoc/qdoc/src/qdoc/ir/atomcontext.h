// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_IR_ATOMCONTEXT_H
#define QDOC_IR_ATOMCONTEXT_H

#include <QJsonObject>
#include <QList>

#include <utility>

QT_BEGIN_NAMESPACE

namespace IR {

struct AtomContext
{
    enum class ContextType : unsigned char {
        Paragraph,
        Brief,
        Section,
        SectionHeading,
        List,
        ListItem,
        Note,
        Warning,
        Important,
        Details,
        CodeBlock,
        Table,
        TableRow,
        TableCell,
        Link,
        Caption,
        Footnote,
        Legalese,
        Quotation,
        Div,
        Sidebar
    };

    struct Frame {
        ContextType type;
        QJsonObject attributes;
    };

    void push(ContextType type, QJsonObject attrs = {})
    {
        m_stack.append(Frame{ type, std::move(attrs) });
    }

    Frame pop()
    {
        Q_ASSERT(!m_stack.isEmpty());
        return m_stack.takeLast();
    }

    [[nodiscard]] bool isInContext(ContextType type) const
    {
        for (auto it = m_stack.crbegin(); it != m_stack.crend(); ++it) {
            if (it->type == type)
                return true;
        }
        return false;
    }

    [[nodiscard]] const Frame &current() const
    {
        Q_ASSERT(!m_stack.isEmpty());
        return m_stack.constLast();
    }

    [[nodiscard]] bool isEmpty() const { return m_stack.isEmpty(); }

    [[nodiscard]] qsizetype depth() const { return m_stack.size(); }

    void clear() { m_stack.clear(); }

private:
    QList<Frame> m_stack;
};

} // namespace IR

QT_END_NAMESPACE

#endif // QDOC_IR_ATOMCONTEXT_H

