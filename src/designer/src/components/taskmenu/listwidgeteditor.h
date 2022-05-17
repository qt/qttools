// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LISTWIDGETEDITOR_H
#define LISTWIDGETEDITOR_H

#include "itemlisteditor.h"
#include <qdesigner_command_p.h>

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QListWidget;
class QComboBox;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ListWidgetEditor: public QDialog
{
    Q_OBJECT

public:
    ListWidgetEditor(QDesignerFormWindowInterface *form,
                     QWidget *parent);

    ListContents fillContentsFromListWidget(QListWidget *listWidget);
    ListContents fillContentsFromComboBox(QComboBox *comboBox);
    ListContents contents() const;

private:
    ItemListEditor *m_itemsEditor;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LISTWIDGETEDITOR_H
