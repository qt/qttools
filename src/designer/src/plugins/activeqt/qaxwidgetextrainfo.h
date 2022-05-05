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

#ifndef ACTIVEQT_EXTRAINFO_H
#define ACTIVEQT_EXTRAINFO_H

#include <QtDesigner/extrainfo.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/default_extensionfactory.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QDesignerAxWidget;

class QAxWidgetExtraInfo: public QObject, public QDesignerExtraInfoExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerExtraInfoExtension)
public:
    QAxWidgetExtraInfo(QDesignerAxWidget *widget, QDesignerFormEditorInterface *core, QObject *parent);

    QWidget *widget() const override;
    QDesignerFormEditorInterface *core() const override;

    bool saveUiExtraInfo(DomUI *ui) override;
    bool loadUiExtraInfo(DomUI *ui) override;

    bool saveWidgetExtraInfo(DomWidget *ui_widget) override;
    bool loadWidgetExtraInfo(DomWidget *ui_widget) override;

private:
    QPointer<QDesignerAxWidget> m_widget;
    QPointer<QDesignerFormEditorInterface> m_core;
};

class QAxWidgetExtraInfoFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    explicit QAxWidgetExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const override;

private:
    QDesignerFormEditorInterface *m_core;
};

QT_END_NAMESPACE

#endif // ACTIVEQT_EXTRAINFO_H
