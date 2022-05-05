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

#ifndef ABSTRACTWIDGETFACTORY_H
#define ABSTRACTWIDGETFACTORY_H

#include <QtDesigner/sdk_global.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QWidget;
class QLayout;

class QDESIGNER_SDK_EXPORT QDesignerWidgetFactoryInterface: public QObject
{
    Q_OBJECT
public:
    explicit QDesignerWidgetFactoryInterface(QObject *parent = nullptr);
    virtual ~QDesignerWidgetFactoryInterface();

    virtual QDesignerFormEditorInterface *core() const = 0;

    virtual QWidget* containerOfWidget(QWidget *w) const = 0;
    virtual QWidget* widgetOfContainer(QWidget *w) const = 0;

    virtual QWidget *createWidget(const QString &name, QWidget *parentWidget = nullptr) const = 0;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const = 0;

    virtual bool isPassiveInteractor(QWidget *widget) = 0;
    virtual void initialize(QObject *object) const = 0;
};

QT_END_NAMESPACE

#endif // ABSTRACTWIDGETFACTORY_H
