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
