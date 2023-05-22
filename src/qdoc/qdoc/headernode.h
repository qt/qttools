// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HEADERNODE_H
#define HEADERNODE_H

#include "aggregate.h"

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class HeaderNode : public Aggregate
{
public:
    HeaderNode(Aggregate *parent, const QString &name);
    [[nodiscard]] bool docMustBeGenerated() const override;
    [[nodiscard]] bool isFirstClassAggregate() const override { return true; }
    [[nodiscard]] bool isRelatableType() const override { return true; }
    [[nodiscard]] QString title() const override { return (m_title.isEmpty() ? name() : m_title); }
    [[nodiscard]] QString subtitle() const override { return m_subtitle; }
    [[nodiscard]] QString fullTitle() const override
    {
        return (m_title.isEmpty() ? name() : name() + " - " + m_title);
    }
    bool setTitle(const QString &title) override
    {
        m_title = title;
        return true;
    }
    bool setSubtitle(const QString &subtitle) override
    {
        m_subtitle = subtitle;
        return true;
    }
    [[nodiscard]] bool hasDocumentedChildren() const;

private:
    QString m_title {};
    QString m_subtitle {};
};

QT_END_NAMESPACE

#endif // HEADERNODE_H
