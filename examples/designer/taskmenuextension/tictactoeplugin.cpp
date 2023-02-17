// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tictactoe.h"
#include "tictactoeplugin.h"
#include "tictactoetaskmenu.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>
#include <QtPlugin>

using namespace Qt::StringLiterals;

//! [0]
TicTacToePlugin::TicTacToePlugin(QObject *parent)
    : QObject(parent)
{
}

QString TicTacToePlugin::name() const
{
    return u"TicTacToe"_s;
}

QString TicTacToePlugin::group() const
{
    return u"Display Widgets [Examples]"_s;
}

QString TicTacToePlugin::toolTip() const
{
    return u"Tic Tac Toe Example, demonstrating class QDesignerTaskMenuExtension (C++)"_s;
}

QString TicTacToePlugin::whatsThis() const
{
    return {};
}

QString TicTacToePlugin::includeFile() const
{
    return u"tictactoe.h"_s;
}

QIcon TicTacToePlugin::icon() const
{
    return {};
}

bool TicTacToePlugin::isContainer() const
{
    return false;
}

QWidget *TicTacToePlugin::createWidget(QWidget *parent)
{
    auto *ticTacToe = new TicTacToe(parent);
    ticTacToe->setState(u"-X-XO----"_s);
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

    auto *manager = formEditor->extensionManager();
    Q_ASSERT(manager != nullptr);
//! [2]

//! [3]
    manager->registerExtensions(new TicTacToeTaskMenuFactory(manager),
                                Q_TYPEID(QDesignerTaskMenuExtension));

    initialized = true;
}

QString TicTacToePlugin::domXml() const
{
    return uR"(
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
)"_s;
}

//! [3]
