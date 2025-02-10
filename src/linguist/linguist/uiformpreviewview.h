// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef UIFORMPREVIEWVIEW_H
#define UIFORMPREVIEWVIEW_H

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

class UiFormPreviewView : public QMainWindow
{
    Q_OBJECT
public:
    UiFormPreviewView(QWidget *parent, MultiDataModel *dataModel);

    void setSourceContext(int model, MessageItem *messageItem);

private:
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

#endif // UIFORMPREVIEWVIEW_H
