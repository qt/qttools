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

#ifndef ENUMITEM_H
#define ENUMITEM_H

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>

#include <utility>

QT_BEGIN_NAMESPACE

class EnumItem
{
public:
    EnumItem() = default;
    EnumItem(QString name, QString value) : m_name(std::move(name)), m_value(std::move(value)) { }

    [[nodiscard]] const QString &name() const { return m_name; }
    [[nodiscard]] const QString &value() const { return m_value; }

private:
    QString m_name {};
    QString m_value {};
};

QT_END_NAMESPACE

#endif // ENUMITEM_H
