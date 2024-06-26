// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "formwindow_dnditem.h"
#include "formwindow.h"

#include <private/ui4_p.h>
#include <qdesigner_resource.h>
#include <qtresourcemodel_p.h>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/private/ui4_p.h>

#include <QtWidgets/qlabel.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

static QWidget *decorationFromWidget(QWidget *w)
{
    QLabel *label = new QLabel(nullptr, Qt::ToolTip);
    QPixmap pm = w->grab(QRect(0, 0, -1, -1));
    label->setPixmap(pm);
    label->resize((QSizeF(pm.size()) / pm.devicePixelRatio()).toSize());

    return label;
}

static DomUI *widgetToDom(QWidget *widget, FormWindow *form)
{
    QDesignerResource builder(form);
    builder.setSaveRelative(false);
    return builder.copy(FormBuilderClipboard(widget));
}

FormWindowDnDItem::FormWindowDnDItem(QDesignerDnDItemInterface::DropType type, FormWindow *form,
                                        QWidget *widget, QPoint global_mouse_pos)
    : QDesignerDnDItem(type, form)
{
    QWidget *decoration = decorationFromWidget(widget);
    QPoint pos = widget->mapToGlobal(QPoint(0, 0));
    decoration->move(pos);

    init(nullptr, widget, decoration, global_mouse_pos);
}

DomUI *FormWindowDnDItem::domUi() const
{
    DomUI *result = QDesignerDnDItem::domUi();
    if (result != nullptr)
        return result;
    FormWindow *form = qobject_cast<FormWindow*>(source());
    if (widget() == nullptr || form == nullptr)
        return nullptr;

    QtResourceModel *resourceModel = form->core()->resourceModel();
    QtResourceSet *currentResourceSet = resourceModel->currentResourceSet();
    /* Short:
     *   We need to activate the original resourceSet associated with a form
     *   to properly generate the dom resource includes.
     * Long:
     *   widgetToDom() calls copy() on QDesignerResource. It generates the
     *   Dom structure. In order to create DomResources properly we need to
     *   have the associated ResourceSet active (QDesignerResource::saveResources()
     *   queries the resource model for a qrc path for the given resource file:
     *      qrcFile = m_core->resourceModel()->qrcPath(ri->text());
     *   This works only when the resource file comes from the active
     *   resourceSet */
    resourceModel->setCurrentResourceSet(form->resourceSet());

    result = widgetToDom(widget(), form);
    const_cast<FormWindowDnDItem*>(this)->setDomUi(result);
    resourceModel->setCurrentResourceSet(currentResourceSet);
    return result;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
