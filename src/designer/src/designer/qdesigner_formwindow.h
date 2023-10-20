// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    bool m_initialized = false;
    bool m_windowTitleInitialized = false;
};

QT_END_NAMESPACE

#endif // QDESIGNER_FORMWINDOW_H
