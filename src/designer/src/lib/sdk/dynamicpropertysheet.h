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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef DYNAMICPROPERTYSHEET_H
#define DYNAMICPROPERTYSHEET_H

#include <QtDesigner/extension.h>

QT_BEGIN_NAMESPACE

class QString; // FIXME: fool syncqt

class QDesignerDynamicPropertySheetExtension
{
public:
    Q_DISABLE_COPY_MOVE(QDesignerDynamicPropertySheetExtension)

    QDesignerDynamicPropertySheetExtension() = default;
    virtual ~QDesignerDynamicPropertySheetExtension() = default;

    virtual bool dynamicPropertiesAllowed() const = 0;
    virtual int addDynamicProperty(const QString &propertyName, const QVariant &value) = 0;
    virtual bool removeDynamicProperty(int index) = 0;
    virtual bool isDynamicProperty(int index) const = 0;
    virtual bool canAddDynamicProperty(const QString &propertyName) const = 0;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerDynamicPropertySheetExtension, "org.qt-project.Qt.Designer.DynamicPropertySheet")

QT_END_NAMESPACE

#endif // DYNAMICPROPERTYSHEET_H
