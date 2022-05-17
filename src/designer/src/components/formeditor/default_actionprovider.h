// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DEFAULT_ACTIONPROVIDER_H
#define DEFAULT_ACTIONPROVIDER_H

#include "formeditor_global.h"
#include "actionprovider_p.h"
#include <extensionfactory_p.h>

#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qtoolbar.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class FormWindow;

class QT_FORMEDITOR_EXPORT ActionProviderBase: public QDesignerActionProviderExtension
{
protected:
    explicit ActionProviderBase(QWidget *widget);

public:
    void adjustIndicator(const QPoint &pos) override;
    virtual Qt::Orientation orientation() const = 0;

protected:
    virtual QRect indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const;

private:
    QWidget *m_indicator;
};

class QT_FORMEDITOR_EXPORT QToolBarActionProvider: public QObject, public ActionProviderBase
{
    Q_OBJECT
    Q_INTERFACES(QDesignerActionProviderExtension)
public:
    explicit QToolBarActionProvider(QToolBar *widget, QObject *parent = nullptr);

    QRect actionGeometry(QAction *action) const override;
    QAction *actionAt(const QPoint &pos) const override;
    Qt::Orientation orientation() const override;

protected:
    QRect indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const override;

private:
    QToolBar *m_widget;
};

class QT_FORMEDITOR_EXPORT QMenuBarActionProvider: public QObject, public ActionProviderBase
{
    Q_OBJECT
    Q_INTERFACES(QDesignerActionProviderExtension)
public:
    explicit QMenuBarActionProvider(QMenuBar *widget, QObject *parent = nullptr);

    QRect actionGeometry(QAction *action) const override;
    QAction *actionAt(const QPoint &pos) const override;
    Qt::Orientation orientation() const override;

private:
    QMenuBar *m_widget;
};

class QT_FORMEDITOR_EXPORT QMenuActionProvider: public QObject, public ActionProviderBase
{
    Q_OBJECT
    Q_INTERFACES(QDesignerActionProviderExtension)
public:
    explicit QMenuActionProvider(QMenu *widget, QObject *parent = nullptr);

    QRect actionGeometry(QAction *action) const override;
    QAction *actionAt(const QPoint &pos) const override;
    Qt::Orientation orientation() const override;

private:
    QMenu *m_widget;
};

using QToolBarActionProviderFactory = ExtensionFactory<QDesignerActionProviderExtension, QToolBar, QToolBarActionProvider>;
using QMenuBarActionProviderFactory = ExtensionFactory<QDesignerActionProviderExtension, QMenuBar, QMenuBarActionProvider>;
using QMenuActionProviderFactory = ExtensionFactory<QDesignerActionProviderExtension, QMenu, QMenuActionProvider>;

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // DEFAULT_ACTIONPROVIDER_H
