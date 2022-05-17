// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMWINDOW_WIDGETSTACK_H
#define FORMWINDOW_WIDGETSTACK_H

#include "formeditor_global.h"

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowToolInterface;

class QStackedLayout;
class QWidget;

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormWindowWidgetStack: public QObject
{
    Q_OBJECT
public:
    FormWindowWidgetStack(QObject *parent = nullptr);
    ~FormWindowWidgetStack() override;

    QLayout *layout() const;

    int count() const;
    QDesignerFormWindowToolInterface *tool(int index) const;
    QDesignerFormWindowToolInterface *currentTool() const;
    int currentIndex() const;
    int indexOf(QDesignerFormWindowToolInterface *tool) const;

    void setMainContainer(QWidget *w = nullptr);

    // Return the widget containing the form which can be used to apply embedded design settings to.
    // These settings should not affect the other editing tools.
    QWidget *formContainer() const { return m_formContainer; }

signals:
    void currentToolChanged(int index);

public slots:
    void addTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(QDesignerFormWindowToolInterface *tool);
    void setCurrentTool(int index);
    void setSenderAsCurrentTool();

protected:
    QWidget *defaultEditor() const;

private:
    QList<QDesignerFormWindowToolInterface*> m_tools;
    QWidget *m_formContainer;
    QStackedLayout *m_formContainerLayout;
    QStackedLayout *m_layout;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOW_WIDGETSTACK_H
