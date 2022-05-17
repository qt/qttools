// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WORLDTIMECLOCKPLUGIN_H
#define WORLDTIMECLOCKPLUGIN_H

#include <QtUiPlugin/QDesignerCustomWidgetInterface>

//! [0]
class WorldTimeClockPlugin : public QObject,
                             public QDesignerCustomWidgetInterface
{
    Q_OBJECT
//! [1]
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
//! [1]
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit WorldTimeClockPlugin(QObject *parent = nullptr);

    bool isContainer() const override;
    bool isInitialized() const override;
    QIcon icon() const override;
    QString domXml() const override;
    QString group() const override;
    QString includeFile() const override;
    QString name() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QWidget *createWidget(QWidget *parent) override;
    void initialize(QDesignerFormEditorInterface *core) override;

private:
    bool initialized = false;
};
//! [0]

#endif
