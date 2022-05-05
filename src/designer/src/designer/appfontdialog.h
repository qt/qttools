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

#ifndef QDESIGNER_APPFONTWIDGET_H
#define QDESIGNER_APPFONTWIDGET_H

#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

class AppFontModel;

class QTreeView;
class QToolButton;
class QItemSelection;
class QDesignerSettingsInterface;

// AppFontWidget: Manages application fonts which the user can load and
// provides API for saving/restoring them.

class AppFontWidget : public QGroupBox
{
    Q_DISABLE_COPY_MOVE(AppFontWidget)
    Q_OBJECT
public:
    explicit AppFontWidget(QWidget *parent = nullptr);

    QStringList fontFiles() const;

    static void save(QDesignerSettingsInterface *s, const QString &prefix);
    static void restore(const QDesignerSettingsInterface *s, const QString &prefix);

private slots:
    void addFiles();
    void slotRemoveFiles();
    void slotRemoveAll();
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

private:
    QTreeView *m_view;
    QToolButton *m_addButton;
    QToolButton *m_removeButton;
    QToolButton *m_removeAllButton;
    AppFontModel *m_model;
};

// AppFontDialog: Non modal dialog for AppFontWidget which has Qt::WA_DeleteOnClose set.

class AppFontDialog : public QDialog
{
    Q_DISABLE_COPY_MOVE(AppFontDialog)
    Q_OBJECT
public:
    explicit AppFontDialog(QWidget *parent = nullptr);

private:
    AppFontWidget *m_appFontWidget;
};

QT_END_NAMESPACE

#endif // QDESIGNER_APPFONTWIDGET_H
