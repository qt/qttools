// Copyright (C) 2016 Stephen Kelly <steveire@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtUiPlugin>
#include <QtUiPlugin/QtUiPlugin>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QDesignerCustomWidgetInterface>

#include <QWidget>

class MyPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    MyPlugin(QObject *parent = 0)
      : QObject(parent)
      , initialized(false)
    {

    }

    bool isContainer() const { return true; }
    bool isInitialized() const { return initialized; }
    QIcon icon() const { return QIcon(); }
    QString domXml() const { return QString(); }
    QString group() const { return QString(); }
    QString includeFile() const { return QString(); }
    QString name() const { return QString(); }
    QString toolTip() const { return QString(); }
    QString whatsThis() const { return QString(); }
    QWidget *createWidget(QWidget *parent) { return new QWidget(parent); }
    void initialize(QDesignerFormEditorInterface *)
    {
        if (initialized)
            return;
        initialized = true;
    }

private:
    bool initialized;
};

#include "my_designer_plugin.moc"
