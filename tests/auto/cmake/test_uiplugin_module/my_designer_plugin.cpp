/****************************************************************************
**
** Copyright (C) 2015 Stephen Kelly <steveire@gmail.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
