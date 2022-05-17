// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tictactoe.h"
#include "tictactoeplugin.h"
#include "tictactoetaskmenu.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>
#include <QtPlugin>

//! [0]
TicTacToePlugin::TicTacToePlugin(QObject *parent)
    : QObject(parent)
{
}

QString TicTacToePlugin::name() const
{
    return QStringLiteral("TicTacToe");
}

QString TicTacToePlugin::group() const
{
    return QStringLiteral("Display Widgets [Examples]");
}

QString TicTacToePlugin::toolTip() const
{
    return QStringLiteral("Tic Tac Toe Example, demonstrating class QDesignerTaskMenuExtension (C++)");
}

QString TicTacToePlugin::whatsThis() const
{
    return QString();
}

QString TicTacToePlugin::includeFile() const
{
    return QStringLiteral("tictactoe.h");
}

QIcon TicTacToePlugin::icon() const
{
    return QIcon();
}

bool TicTacToePlugin::isContainer() const
{
    return false;
}

QWidget *TicTacToePlugin::createWidget(QWidget *parent)
{
    TicTacToe *ticTacToe = new TicTacToe(parent);
    ticTacToe->setState(QStringLiteral("-X-XO----"));
    return ticTacToe;
}

bool TicTacToePlugin::isInitialized() const
{
    return initialized;
}

//! [0] //! [1]
void TicTacToePlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
//! [1] //! [2]
    if (initialized)
        return;

    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);
//! [2]

//! [3]
    manager->registerExtensions(new TicTacToeTaskMenuFactory(manager),
                                Q_TYPEID(QDesignerTaskMenuExtension));

    initialized = true;
}

QString TicTacToePlugin::domXml() const
{
    return QLatin1String(R"(
<ui language="c++">
    <widget class="TicTacToe" name="ticTacToe"/>
    <customwidgets>
        <customwidget>
            <class>TicTacToe</class>
            <propertyspecifications>
            <tooltip name="state">Tic Tac Toe state</tooltip>
            <stringpropertyspecification name="state" notr="true" type="singleline"/>
            </propertyspecifications>
        </customwidget>
    </customwidgets>
</ui>
)");
}

//! [3]
