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
******************************************************************************/

#ifndef ABSTRACTSETTINGS_P_H
#define ABSTRACTSETTINGS_P_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QString;

class QDESIGNER_SDK_EXPORT QDesignerSettingsInterface
{
public:
    Q_DISABLE_COPY_MOVE(QDesignerSettingsInterface)

    QDesignerSettingsInterface() = default;
    virtual ~QDesignerSettingsInterface() = default;

    virtual void beginGroup(const QString &prefix) = 0;
    virtual void endGroup() = 0;

    virtual bool contains(const QString &key) const = 0;
    virtual void setValue(const QString &key, const QVariant &value) = 0;
    virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const = 0;
    virtual void remove(const QString &key) = 0;
};

QT_END_NAMESPACE

#endif // ABSTRACTSETTINGS_P_H
