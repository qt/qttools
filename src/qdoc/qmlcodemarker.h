// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLCODEMARKER_H
#define QMLCODEMARKER_H

#include "cppcodemarker.h"

#include <private/qqmljsastfwd_p.h>

QT_BEGIN_NAMESPACE

class QmlCodeMarker : public CppCodeMarker
{
public:
    QmlCodeMarker() = default;
    ~QmlCodeMarker() override = default;

    bool recognizeCode(const QString &code) override;
    bool recognizeExtension(const QString &ext) override;
    bool recognizeLanguage(const QString &language) override;
    [[nodiscard]] Atom::AtomType atomType() const override;
    QString markedUpCode(const QString &code, const Node *relative,
                         const Location &location) override;

    QString markedUpName(const Node *node) override;
    QString markedUpInclude(const QString &include) override;

    /* Copied from src/declarative/qml/qdeclarativescriptparser.cpp */
    QList<QQmlJS::SourceLocation> extractPragmas(QString &script);

private:
    QString addMarkUp(const QString &code, const Node *relative, const Location &location);
};

QT_END_NAMESPACE

#endif
