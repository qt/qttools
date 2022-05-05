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

#ifndef ABSTRACTOPTIONSPAGE_P_H
#define ABSTRACTOPTIONSPAGE_P_H

#include <QtDesigner/sdk_global.h>

QT_BEGIN_NAMESPACE

class QString;
class QWidget;

class QDESIGNER_SDK_EXPORT QDesignerOptionsPageInterface
{
public:
    Q_DISABLE_COPY_MOVE(QDesignerOptionsPageInterface)

    QDesignerOptionsPageInterface() = default;
    virtual ~QDesignerOptionsPageInterface() = default;

    virtual QString name() const = 0;
    virtual QWidget *createPage(QWidget *parent) = 0;
    virtual void apply() = 0;
    virtual void finish() = 0;
};

QT_END_NAMESPACE

#endif // ABSTRACTOPTIONSPAGE_P_H
