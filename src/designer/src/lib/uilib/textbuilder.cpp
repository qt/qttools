// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
