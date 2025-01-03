// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef WIDGETBOX_DNDITEM_H
#define WIDGETBOX_DNDITEM_H

#include <qdesigner_dnditem_p.h>
#include "widgetbox_global.h"

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class DomUI;

namespace qdesigner_internal {

class QT_WIDGETBOX_EXPORT WidgetBoxDnDItem : public QDesignerDnDItem
{
public:
    WidgetBoxDnDItem(QDesignerFormEditorInterface *core,
                     DomUI *dom_ui,
                     const QPoint &global_mouse_pos);
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETBOX_DNDITEM_H
