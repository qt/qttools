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

#ifndef QDESIGNER_FORMWINDOW_H
#define QDESIGNER_FORMWINDOW_H

#include <QtCore/qpointer.h>
#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QDesignerWorkbench;
class QDesignerFormWindowInterface;

class QDesignerFormWindow: public QWidget
{
    Q_OBJECT
public:
    QDesignerFormWindow(QDesignerFormWindowInterface *formWindow, QDesignerWorkbench *workbench,
                        QWidget *parent = nullptr, Qt::WindowFlags flags = {});

    void firstShow();

    ~QDesignerFormWindow() override;

    QAction *action() const;
    QDesignerWorkbench *workbench() const;
    QDesignerFormWindowInterface *editor() const;

    QRect geometryHint() const;

public slots:
    void updateChanged();

private slots:
    void updateWindowTitle(const QString &fileName);
    void slotGeometryChanged();

signals:
    void minimizationStateChanged(QDesignerFormWindowInterface *formWindow, bool minimized);
    void triggerAction();

protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *ev) override;
    void resizeEvent(QResizeEvent* rev) override;

private:
    int getNumberOfUntitledWindows() const;
    QPointer<QDesignerFormWindowInterface> m_editor;
    QPointer<QDesignerWorkbench> m_workbench;
    QAction *m_action;
    bool m_initialized;
    bool m_windowTitleInitialized;
};

QT_END_NAMESPACE

#endif // QDESIGNER_FORMWINDOW_H
