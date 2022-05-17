// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
