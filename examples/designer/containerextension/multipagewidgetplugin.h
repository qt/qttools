// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
#ifndef MULTIPAGEWIDGETPLUGIN_H
#define MULTIPAGEWIDGETPLUGIN_H

#include <QtUiPlugin/QDesignerCustomWidgetInterface>

QT_BEGIN_NAMESPACE
class QIcon;
class QWidget;
QT_END_NAMESPACE

class MultiPageWidgetPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
//! [1]
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidget")
//! [1]
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    explicit MultiPageWidgetPlugin(QObject *parent = nullptr);

    QString name() const override;
    QString group() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QString includeFile() const override;
    QIcon icon() const override;
    bool isContainer() const override;
    QWidget *createWidget(QWidget *parent) override;
    bool isInitialized() const override;
    void initialize(QDesignerFormEditorInterface *formEditor) override;
    QString domXml() const override;

private slots:
    void currentIndexChanged(int index);
    void pageTitleChanged(const QString &title);

private:
    bool initialized = false;
};

#endif
//! [0]
