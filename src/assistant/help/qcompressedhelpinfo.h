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

#ifndef QCOMPRESSEDHELPINFO_H
#define QCOMPRESSEDHELPINFO_H

#include <QtHelp/qhelp_global.h>

#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QVersionNumber;
class QCompressedHelpInfoPrivate;

class QHELP_EXPORT QCompressedHelpInfo final
{
public:
    QCompressedHelpInfo();
    QCompressedHelpInfo(const QCompressedHelpInfo &other);
    QCompressedHelpInfo(QCompressedHelpInfo &&other);
    ~QCompressedHelpInfo();

    QCompressedHelpInfo &operator=(const QCompressedHelpInfo &other);
    QCompressedHelpInfo &operator=(QCompressedHelpInfo &&other);

    void swap(QCompressedHelpInfo &other) Q_DECL_NOTHROW
    { d.swap(other.d); }

    QString namespaceName() const;
    QString component() const;
    QVersionNumber version() const;
    bool isNull() const;

    static QCompressedHelpInfo fromCompressedHelpFile(const QString &documentationFileName);

private:
    QSharedDataPointer<QCompressedHelpInfoPrivate> d;
};

QT_END_NAMESPACE

#endif // QHELPCOLLECTIONDETAILS_H
