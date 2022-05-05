/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#include "textbuilder_p.h"
#include "ui4_p.h"
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal {
#endif

QTextBuilder::QTextBuilder() = default;

QTextBuilder::~QTextBuilder() = default;

QVariant QTextBuilder::loadText(const DomProperty *property) const
{
    if (property->kind() == DomProperty::String)
        return property->elementString()->text();
    return QVariant();
}

QVariant QTextBuilder::toNativeValue(const QVariant &value) const
{
    return value;
}

DomProperty *QTextBuilder::saveText(const QVariant &value) const
{
    Q_UNUSED(value);
    return nullptr;
}

#ifdef QFORMINTERNAL_NAMESPACE
} // namespace QFormInternal
#endif

QT_END_NAMESPACE
