// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMAINWINDOW_CONTAINER_H
#define QMAINWINDOW_CONTAINER_H

#include <QtDesigner/container.h>
#include <QtDesigner/default_extensionfactory.h>

#include <extensionfactory_p.h>

#include <QtWidgets/qmainwindow.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QMainWindowContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QMainWindowContainer(QMainWindow *widget, QObject *parent = nullptr);

    int count() const override;
    QWidget *widget(int index) const override;
    int currentIndex() const override;
    void setCurrentIndex(int index) override;
    bool canAddWidget() const override { return true; }
    void addWidget(QWidget *widget) override;
    void insertWidget(int index, QWidget *widget) override;
    bool canRemove(int) const override { return true; }
    void remove(int index) override;

private:
    QMainWindow *m_mainWindow;
    QWidgetList m_widgets;
};

using QMainWindowContainerFactory = ExtensionFactory<QDesignerContainerExtension, QMainWindow, QMainWindowContainer>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QMAINWINDOW_CONTAINER_H
