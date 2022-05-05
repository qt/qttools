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
