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
    QString markedUpIncludes(const QStringList &includes) override;
    QString functionBeginRegExp(const QString &funcName) override;
    QString functionEndRegExp(const QString &funcName) override;

private:
    QString addMarkUp(const QString &protectedCode, const Node *relative, const Location &location);
};

QT_END_NAMESPACE

#endif
