// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LAYOUT_PROPERTYSHEET_H
#define LAYOUT_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>

#include <QtWidgets/qlayout.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class DomLayout;

namespace qdesigner_internal {

class LayoutPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit LayoutPropertySheet(QLayout *object, QObject *parent = nullptr);
    ~LayoutPropertySheet() override;

    void setProperty(int index, const QVariant &value) override;
    QVariant property(int index) const override;
    bool reset(int index) override;
    void setChanged(int index, bool changed) override;

    static void stretchAttributesToDom(QDesignerFormEditorInterface *core, QLayout *lt, DomLayout *domLayout);
    static void markChangedStretchProperties(QDesignerFormEditorInterface *core, QLayout *lt, const DomLayout *domLayout);

private:
    QLayout *m_layout;
};

using LayoutPropertySheetFactory = QDesignerPropertySheetFactory<QLayout, LayoutPropertySheet>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LAYOUT_PROPERTYSHEET_H
