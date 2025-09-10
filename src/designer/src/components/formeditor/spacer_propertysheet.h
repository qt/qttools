// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
