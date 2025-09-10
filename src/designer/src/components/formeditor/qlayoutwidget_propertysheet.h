// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QLAYOUTWIDGET_PROPERTYSHEET_H
#define QLAYOUTWIDGET_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>
#include <qlayout_widget_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QLayoutWidgetPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent = nullptr);
    ~QLayoutWidgetPropertySheet() override;

    void setProperty(int index, const QVariant &value) override;
    bool isVisible(int index) const override;

    bool dynamicPropertiesAllowed() const override;
};

using QLayoutWidgetPropertySheetFactory = QDesignerPropertySheetFactory<QLayoutWidget, QLayoutWidgetPropertySheet>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QLAYOUTWIDGET_PROPERTYSHEET_H
