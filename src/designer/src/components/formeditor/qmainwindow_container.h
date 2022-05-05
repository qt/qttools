/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
