/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#ifndef FORMPREVIEWVIEW_H
#define FORMPREVIEWVIEW_H

#include <private/quiloader_p.h>

#include <QtCore/QHash>
#include <QtCore/QList>

#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE

class MultiDataModel;
class FormFrame;
class MessageItem;

class QComboBox;
class QListWidgetItem;
class QGridLayout;
class QMdiArea;
class QMdiSubWindow;
class QToolBox;
class QTableWidgetItem;
class QTreeWidgetItem;

enum TranslatableEntryType {
    TranslatableProperty,
    TranslatableToolItemText,
    TranslatableToolItemToolTip,
    TranslatableTabPageText,
    TranslatableTabPageToolTip,
    TranslatableTabPageWhatsThis,
    TranslatableListWidgetItem,
    TranslatableTableWidgetItem,
    TranslatableTreeWidgetItem,
    TranslatableComboBoxItem
};

struct TranslatableEntry {
    TranslatableEntryType type;
    union {
        QObject *object;
        QComboBox *comboBox;
        QTabWidget *tabWidget;
        QToolBox *toolBox;
        QListWidgetItem *listWidgetItem;
        QTableWidgetItem *tableWidgetItem;
        QTreeWidgetItem *treeWidgetItem;
    } target;
    union {
        char *name;
        int index;
        struct {
            short index; // Known to be below 1000
            short column;
        } treeIndex;
    } prop;
};

typedef QHash<QUiTranslatableStringValue, QList<TranslatableEntry> > TargetsHash;

class FormPreviewView : public QMainWindow
{
    Q_OBJECT
public:
    FormPreviewView(QWidget *parent, MultiDataModel *dataModel);

    void setSourceContext(int model, MessageItem *messageItem);

private:
    bool m_isActive;
    QString m_currentFileName;
    QMdiArea *m_mdiArea;
    QMdiSubWindow *m_mdiSubWindow;
    QWidget *m_form;
    TargetsHash m_targets;
    QList<TranslatableEntry> m_highlights;
    MultiDataModel *m_dataModel;

    QString m_lastFormName;
    QString m_lastClassName;
    int m_lastModel;
};

QT_END_NAMESPACE

#endif // FORMPREVIEWVIEW_H
