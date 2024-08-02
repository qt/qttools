// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CPPCODEMARKER_H
#define CPPCODEMARKER_H

#include "codemarker.h"

QT_BEGIN_NAMESPACE

class CppCodeMarker : public CodeMarker
{
public:
    CppCodeMarker() = default;
    ~CppCodeMarker() override = default;

    bool recognizeCode(const QString &code) override;
    bool recognizeExtension(const QString &ext) override;
    bool recognizeLanguage(const QString &lang) override;
    [[nodiscard]] Atom::AtomType atomType() const override;
    QString markedUpCode(const QString &code, const Node *relative,
                         const Location &location) override;
    QString markedUpSynopsis(const Node *node, const Node *relative, Section::Style style) override;
    QString markedUpQmlItem(const Node *node, bool summary) override;
    QString markedUpName(const Node *node) override;
    QString markedUpEnumValue(const QString &enumValue, const Node *relative) override;

private:
    QString addMarkUp(const QString &protectedCode, const Node *relative, const Location &location);
};

QT_END_NAMESPACE

#endif
