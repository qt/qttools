// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FORMWINDOW_DNDITEM_H
#define FORMWINDOW_DNDITEM_H

#include <qdesigner_dnditem_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class FormWindow;

class FormWindowDnDItem : public QDesignerDnDItem
{
public:
    FormWindowDnDItem(QDesignerDnDItemInterface::DropType type, FormWindow *form,
                      QWidget *widget, QPoint global_mouse_pos);
    DomUI *domUi() const override;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOW_DNDITEM_H
