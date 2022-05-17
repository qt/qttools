// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MULTIPAGEWIDGETCONTAINEREXTENSION_H
#define MULTIPAGEWIDGETCONTAINEREXTENSION_H

#include <QtDesigner/QDesignerContainerExtension>

QT_BEGIN_NAMESPACE
class QExtensionManager;
QT_END_NAMESPACE
class MultiPageWidget;

//! [0]
class MultiPageWidgetContainerExtension: public QObject,
                                         public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)

public:
    explicit MultiPageWidgetContainerExtension(MultiPageWidget *widget, QObject *parent);

    bool canAddWidget() const override;
    void addWidget(QWidget *widget) override;
    int count() const override;
    int currentIndex() const override;
    void insertWidget(int index, QWidget *widget) override;
    bool canRemove(int index) const override;
    void remove(int index) override;
    void setCurrentIndex(int index) override;
    QWidget *widget(int index) const override;

private:
    MultiPageWidget *myWidget;
};
//! [0]

#endif
