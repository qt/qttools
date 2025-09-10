// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKWIDGET_PLUGIN_H
#define QQUICKWIDGET_PLUGIN_H

#include <QtQuick/qquickwindow.h>
#include <QtUiPlugin/customwidget.h>

QT_BEGIN_NAMESPACE

class QQuickWidgetPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    explicit QQuickWidgetPlugin(QObject *parent = nullptr);

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

private slots:
    void sceneGraphError(QQuickWindow::SceneGraphError, const QString &);

private:
    bool m_initialized = false;
};

QT_END_NAMESPACE

#endif // QQUICKWIDGET_PLUGIN_H
