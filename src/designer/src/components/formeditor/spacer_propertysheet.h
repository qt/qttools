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

#ifndef SPACER_PROPERTYSHEET_H
#define SPACER_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>
#include <spacer_widget_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class SpacerPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit SpacerPropertySheet(Spacer *object, QObject *parent = nullptr);
     ~SpacerPropertySheet() override;

    void setProperty(int index, const QVariant &value) override;
    bool isVisible(int index) const override;

    bool dynamicPropertiesAllowed() const override;
};

using SpacerPropertySheetFactory = QDesignerPropertySheetFactory<Spacer, SpacerPropertySheet>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SPACER_PROPERTYSHEET_H
