// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "multipagewidgetcontainerextension.h"
#include "multipagewidget.h"

//! [0]
MultiPageWidgetContainerExtension::MultiPageWidgetContainerExtension(MultiPageWidget *widget,
                                                                     QObject *parent)
    : QObject(parent)
    , myWidget(widget)
{
}
//! [0]

//! [1]
bool MultiPageWidgetContainerExtension::canAddWidget() const
{
    return true;
}

void MultiPageWidgetContainerExtension::addWidget(QWidget *widget)
{
    myWidget->addPage(widget);
}
//! [1]

//! [2]
int MultiPageWidgetContainerExtension::count() const
{
    return myWidget->count();
}
//! [2]

//! [3]
int MultiPageWidgetContainerExtension::currentIndex() const
{
    return myWidget->currentIndex();
}
//! [3]

//! [4]
void MultiPageWidgetContainerExtension::insertWidget(int index, QWidget *widget)
{
    myWidget->insertPage(index, widget);
}
//! [4]

//! [5]
bool MultiPageWidgetContainerExtension::canRemove(int index) const
{
    Q_UNUSED(index);
    return true;
}

void MultiPageWidgetContainerExtension::remove(int index)
{
    myWidget->removePage(index);
}
//! [5]

//! [6]
void MultiPageWidgetContainerExtension::setCurrentIndex(int index)
{
    myWidget->setCurrentIndex(index);
}
//! [6]

//! [7]
QWidget* MultiPageWidgetContainerExtension::widget(int index) const
{
    return myWidget->widget(index);
}
//! [7]
