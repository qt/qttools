/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QMLCODEMARKER_H
#define QMLCODEMARKER_H

#include "cppcodemarker.h"

#ifndef QT_NO_DECLARATIVE
#    include <private/qqmljsastfwd_p.h>
#endif

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
    QString markedUpIncludes(const QStringList &includes) override;
    QString functionBeginRegExp(const QString &funcName) override;
    QString functionEndRegExp(const QString &funcName) override;

    /* Copied from src/declarative/qml/qdeclarativescriptparser.cpp */
#ifndef QT_NO_DECLARATIVE
    QList<QQmlJS::SourceLocation> extractPragmas(QString &script);
#endif

private:
    QString addMarkUp(const QString &code, const Node *relative, const Location &location);
};

QT_END_NAMESPACE

#endif
