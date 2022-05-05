/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef GRADIENTVIEW_H
#define GRADIENTVIEW_H

#include <QtWidgets/QWidget>
#include <QtCore/QMap>
#include "ui_qtgradientview.h"

QT_BEGIN_NAMESPACE

class QtGradientManager;
class QListViewItem;
class QAction;

class QtGradientView : public QWidget
{
    Q_OBJECT
public:
    QtGradientView(QWidget *parent = 0);

    void setGradientManager(QtGradientManager *manager);
    QtGradientManager *gradientManager() const;

    void setCurrentGradient(const QString &id);
    QString currentGradient() const;

signals:
    void currentGradientChanged(const QString &id);
    void gradientActivated(const QString &id);

private slots:
    void slotGradientAdded(const QString &id, const QGradient &gradient);
    void slotGradientRenamed(const QString &id, const QString &newId);
    void slotGradientChanged(const QString &id, const QGradient &newGradient);
    void slotGradientRemoved(const QString &id);
    void slotNewGradient();
    void slotEditGradient();
    void slotRemoveGradient();
    void slotRenameGradient();
    void slotRenameGradientItem(QListWidgetItem *item);
    void slotCurrentItemChanged(QListWidgetItem *item);
    void slotGradientActivated(QListWidgetItem *item);

private:
    QMap<QString, QListWidgetItem *> m_idToItem;
    QMap<QListWidgetItem *, QString> m_itemToId;

    QAction *m_newAction;
    QAction *m_editAction;
    QAction *m_renameAction;
    QAction *m_removeAction;

    QtGradientManager *m_manager;
    Ui::QtGradientView m_ui;
};

QT_END_NAMESPACE

#endif
