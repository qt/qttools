// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
