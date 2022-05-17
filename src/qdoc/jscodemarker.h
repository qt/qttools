// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef JSCODEMARKER_H
#define JSCODEMARKER_H

#include "qmlcodemarker.h"

QT_BEGIN_NAMESPACE

class JsCodeMarker : public QmlCodeMarker
{
public:
    JsCodeMarker() = default;
    ~JsCodeMarker() override = default;

    bool recognizeCode(const QString &code) override;
    bool recognizeExtension(const QString &ext) override;
    bool recognizeLanguage(const QString &language) override;
    [[nodiscard]] Atom::AtomType atomType() const override;

    QString markedUpCode(const QString &code, const Node *relative,
                         const Location &location) override;

private:
    QString addMarkUp(const QString &code, const Node *relative, const Location &location);
};

QT_END_NAMESPACE

#endif
