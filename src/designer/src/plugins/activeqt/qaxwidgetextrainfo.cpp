// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qaxwidgetextrainfo.h"
#include "qdesigneraxwidget.h"
#include "qaxwidgetpropertysheet.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/private/ui4_p.h>

QT_BEGIN_NAMESPACE

QAxWidgetExtraInfo::QAxWidgetExtraInfo(QDesignerAxWidget *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{
}

QWidget *QAxWidgetExtraInfo::widget() const
{
    return m_widget;
}

QDesignerFormEditorInterface *QAxWidgetExtraInfo::core() const
{
    return m_core;
}

bool QAxWidgetExtraInfo::saveUiExtraInfo(DomUI *)
{
    return false;
}

bool QAxWidgetExtraInfo::loadUiExtraInfo(DomUI *)
{
    return false;
}

bool QAxWidgetExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    /* Turn off standard setters and make sure "control" is in front,
     * otherwise, previews will not work as the properties are not applied via
     * the caching property sheet, them. */
    QList<DomProperty *> props = ui_widget->elementProperty();
    const qsizetype size = props.size();
    const QString controlProperty = QLatin1String(QAxWidgetPropertySheet::controlPropertyName);
    for (qsizetype i = 0; i < size; ++i) {
        props.at(i)->setAttributeStdset(false);
        if (i > 0 && props.at(i)->attributeName() == controlProperty) {
            qSwap(props[0], props[i]);
            ui_widget->setElementProperty(props);
        }

    }
    return true;
}

bool QAxWidgetExtraInfo::loadWidgetExtraInfo(DomWidget *)
{
    return false;
}

QAxWidgetExtraInfoFactory::QAxWidgetExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{
}

QObject *QAxWidgetExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (QDesignerAxWidget *w = qobject_cast<QDesignerAxWidget*>(object))
        return new QAxWidgetExtraInfo(w, m_core, parent);

    return 0;
}

QT_END_NAMESPACE
