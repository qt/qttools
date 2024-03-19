// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QACTIVEXTASKMENU_H
#define QACTIVEXTASKMENU_H

#include <QtDesigner/taskmenu.h>
#include <QtDesigner/private/extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QDesignerAxWidget;

class QAxWidgetTaskMenu: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit QAxWidgetTaskMenu(QDesignerAxWidget *object, QObject *parent = nullptr);
    virtual ~QAxWidgetTaskMenu();
    QList<QAction*> taskActions() const override;

private slots:
    void setActiveXControl();
    void resetActiveXControl();

private:
    QDesignerAxWidget *m_axwidget;
    QAction *m_setAction;
    QAction *m_resetAction;
    QList<QAction*> m_taskActions;
};

using ActiveXTaskMenuFactory = qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, QDesignerAxWidget, QAxWidgetTaskMenu>;

QT_END_NAMESPACE

#endif // QACTIVEXTASKMENU
