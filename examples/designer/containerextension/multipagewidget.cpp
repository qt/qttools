// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QComboBox>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "multipagewidget.h"

MultiPageWidget::MultiPageWidget(QWidget *parent)
    : QWidget(parent)
    , stackWidget(new QStackedWidget)
    , comboBox(new QComboBox)
{
    comboBox->setObjectName("__qt__passive_comboBox");

    connect(comboBox, &QComboBox::activated,
            this, &MultiPageWidget::setCurrentIndex);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(comboBox);
    layout->addWidget(stackWidget);
}

QSize MultiPageWidget::sizeHint() const
{
    return {200, 150};
}

void MultiPageWidget::addPage(QWidget *page)
{
    insertPage(count(), page);
}

void MultiPageWidget::removePage(int index)
{
    auto *widget = stackWidget->widget(index);
    stackWidget->removeWidget(widget);

    comboBox->removeItem(index);
}

int MultiPageWidget::count() const
{
    return stackWidget->count();
}

int MultiPageWidget::currentIndex() const
{
    return stackWidget->currentIndex();
}

void MultiPageWidget::insertPage(int index, QWidget *page)
{
    page->setParent(stackWidget);

    stackWidget->insertWidget(index, page);

    QString title = page->windowTitle();
    if (title.isEmpty()) {
        title = tr("Page %1").arg(comboBox->count() + 1);
        page->setWindowTitle(title);
    }
    connect(page, &QWidget::windowTitleChanged,
            this, &MultiPageWidget::pageWindowTitleChanged);
    comboBox->insertItem(index, title);
}

void MultiPageWidget::setCurrentIndex(int index)
{
    if (index != currentIndex()) {
        stackWidget->setCurrentIndex(index);
        comboBox->setCurrentIndex(index);
        emit currentIndexChanged(index);
    }
}

void MultiPageWidget::pageWindowTitleChanged()
{
    QWidget *page = qobject_cast<QWidget *>(sender());
    const int index = stackWidget->indexOf(page);
    comboBox->setItemText(index, page->windowTitle());
}

QWidget* MultiPageWidget::widget(int index)
{
    return stackWidget->widget(index);
}

QString MultiPageWidget::pageTitle() const
{
    if (const QWidget *currentWidget = stackWidget->currentWidget())
        return currentWidget->windowTitle();
    return {};
}

void MultiPageWidget::setPageTitle(const QString &newTitle)
{
    if (QWidget *currentWidget = stackWidget->currentWidget())
        currentWidget->setWindowTitle(newTitle);
    emit pageTitleChanged(newTitle);
}
