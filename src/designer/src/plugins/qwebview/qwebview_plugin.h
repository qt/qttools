// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWEBPAGE_PLUGIN_H
#define QWEBPAGE_PLUGIN_H

#include <QtUiPlugin/customwidget.h>

QT_BEGIN_NAMESPACE

class QWebViewPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface" FILE "qwebview.json")
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    QWebViewPlugin(QObject *parent = 0);

    QString name() const override;
    QString group() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QString includeFile() const override;
    QIcon icon() const override;
    bool isContainer() const override;
    QWidget *createWidget(QWidget *parent) override;
    bool isInitialized() const override;
    void initialize(QDesignerFormEditorInterface *core) override;
    QString domXml() const override;

private:
    bool m_initialized;
};

QT_END_NAMESPACE

#endif // QWEBPAGE_PLUGIN_H
