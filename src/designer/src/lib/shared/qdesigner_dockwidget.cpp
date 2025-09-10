// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_dockwidget_p.h"
#include "layoutinfo_p.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/container.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractformwindowcursor.h>

#include <qdesigner_propertysheet_p.h>

#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qlayout.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

bool QDockWidgetPropertySheet::isEnabled(int index) const
{
    const QString &name = propertyName(index);
    if (name == "dockWidgetArea"_L1)
        return static_cast<const QDesignerDockWidget *>(object())->docked();
    if (name == "docked"_L1)
        return static_cast<const QDesignerDockWidget *>(object())->inMainWindow();
    return QDesignerPropertySheet::isEnabled(index);
}

QDesignerDockWidget::QDesignerDockWidget(QWidget *parent)
    : QDockWidget(parent)
{
}

QDesignerDockWidget::~QDesignerDockWidget() = default;

bool QDesignerDockWidget::docked() const
{
    return qobject_cast<QMainWindow*>(parentWidget()) != 0;
}

void QDesignerDockWidget::setDocked(bool b)
{
    if (QMainWindow *mainWindow = findMainWindow()) {
        QDesignerFormEditorInterface *core = formWindow()->core();
        QDesignerContainerExtension *c;
        c = qt_extension<QDesignerContainerExtension*>(core->extensionManager(), mainWindow);
        if (b && !docked()) {
            // Dock it
            // ### undo/redo stack
            setParent(nullptr);
            c->addWidget(this);
            formWindow()->selectWidget(this, formWindow()->cursor()->isWidgetSelected(this));
        } else if (!b && docked()) {
            // Undock it
            for (int i = 0; i < c->count(); ++i) {
                if (c->widget(i) == this) {
                    c->remove(i);
                    break;
                }
            }
            // #### restore the position
            setParent(mainWindow->centralWidget());
            show();
            formWindow()->selectWidget(this, formWindow()->cursor()->isWidgetSelected(this));
        }
    }
}

Qt::DockWidgetArea QDesignerDockWidget::dockWidgetArea() const
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget()))
        return mainWindow->dockWidgetArea(const_cast<QDesignerDockWidget*>(this));

    return Qt::LeftDockWidgetArea;
}

void QDesignerDockWidget::setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea)
{
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget())) {
        if ((dockWidgetArea != Qt::NoDockWidgetArea)
            && isAreaAllowed(dockWidgetArea)) {
            mainWindow->addDockWidget(dockWidgetArea, this);
        }
    }
}

bool QDesignerDockWidget::inMainWindow() const
{
    QMainWindow *mw = findMainWindow();
    if (mw && !mw->centralWidget()->layout()) {
        if (mw == parentWidget())
            return true;
        if (mw->centralWidget() == parentWidget())
            return true;
    }
    return false;
}

QDesignerFormWindowInterface *QDesignerDockWidget::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerDockWidget*>(this));
}

QMainWindow *QDesignerDockWidget::findMainWindow() const
{
    if (QDesignerFormWindowInterface *fw = formWindow())
        return qobject_cast<QMainWindow*>(fw->mainContainer());
    return nullptr;
}

QT_END_NAMESPACE
