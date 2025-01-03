// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QACTIVEXPROPERTYSHEET_H
#define QACTIVEXPROPERTYSHEET_H

#include <QtDesigner/private/qdesigner_propertysheet_p.h>

QT_BEGIN_NAMESPACE

class QDesignerAxWidget;
class QDesignerFormWindowInterface;

/* The propertysheet has a method to delete itself and repopulate
 * if the "control" property changes. Pre 4.5, the control property
 * might not be the first one, so, the properties are stored and
 * re-applied. If the "control" is the first one, it should be
 * sufficient to reapply the changed flags, however, care must be taken when
 * resetting the control.
 * Resetting a control: The current behaviour is that the modified Active X properties are added again
 * as Fake-Properties, which is a nice side-effect as not cause a loss. */

class QAxWidgetPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit QAxWidgetPropertySheet(QDesignerAxWidget *object, QObject *parent = 0);

    bool isEnabled(int index) const override;
    bool isVisible(int index) const override;
    QVariant property(int index) const override;
    void setProperty(int index, const QVariant &value) override;
    bool reset(int index) override;
    int indexOf(const QString &name) const override;
    bool dynamicPropertiesAllowed() const override;

    static const char *controlPropertyName;

public slots:
    void updatePropertySheet();

private:
    QDesignerAxWidget *axWidget() const;

    const QString m_controlProperty;
    const QString m_propertyGroup;
    int m_controlIndex;
    struct SavedProperties {
        QVariantMap changedProperties;
        QWidget *widget;
        QString clsid;
    } m_currentProperties;

    static void reloadPropertySheet(const struct SavedProperties &properties, QDesignerFormWindowInterface *formWin);
};

using ActiveXPropertySheetFactory = QDesignerPropertySheetFactory<QDesignerAxWidget, QAxWidgetPropertySheet>;

QT_END_NAMESPACE

#endif
