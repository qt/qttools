// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LINE_PROPERTYSHEET_H
#define LINE_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <qdesigner_widget_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class LinePropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit LinePropertySheet(Line *object, QObject *parent = nullptr);
    ~LinePropertySheet() override;

    void setProperty(int index, const QVariant &value) override;
    bool isVisible(int index) const override;
    QString propertyGroup(int index) const override;
};

using LinePropertySheetFactory = QDesignerPropertySheetFactory<Line, LinePropertySheet>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LINE_PROPERTYSHEET_H
