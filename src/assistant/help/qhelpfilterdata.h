/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#ifndef QHELPFILTERDATA_H
#define QHELPFILTERDATA_H

#include <QtHelp/qhelp_global.h>

#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QVersionNumber;
class QHelpFilterDataPrivate;

class QHELP_EXPORT QHelpFilterData final
{
public:
    QHelpFilterData();
    QHelpFilterData(const QHelpFilterData &other);
    QHelpFilterData(QHelpFilterData &&other);
    ~QHelpFilterData();

    QHelpFilterData &operator=(const QHelpFilterData &other);
    QHelpFilterData &operator=(QHelpFilterData &&other);
    bool operator==(const QHelpFilterData &other) const;

    void swap(QHelpFilterData &other) Q_DECL_NOTHROW
    { d.swap(other.d); }

    void setComponents(const QStringList &components);
    void setVersions(const QList<QVersionNumber> &versions);

    QStringList components() const;
    QList<QVersionNumber> versions() const;
private:
    QSharedDataPointer<QHelpFilterDataPrivate> d;
};

QT_END_NAMESPACE

#endif // QHELPFILTERDATA_H
