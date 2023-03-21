// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "listwidgeteditor.h"
#include <designerpropertymanager.h>
#include <abstractformbuilder.h>

#include <QtDesigner/abstractsettings.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qdialogbuttonbox.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

ListWidgetEditor::ListWidgetEditor(QDesignerFormWindowInterface *form,
                                   QWidget *parent)
    : QDialog(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    m_itemsEditor = new ItemListEditor(form, nullptr);
    m_itemsEditor->layout()->setContentsMargins(QMargins());
    m_itemsEditor->setNewItemText(tr("New Item"));

    QFrame *sep = new QFrame;
    sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    QBoxLayout *box = new QVBoxLayout(this);
    box->addWidget(m_itemsEditor);
    box->addWidget(sep);
    box->addWidget(buttonBox);

    // Numbers copied from itemlisteditor.ui
    // (Automatic resizing doesn't work because ui has parent).
    resize(550, 360);
}

static AbstractItemEditor::PropertyDefinition listBoxPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { Qt::ToolTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "toolTip" },
    { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
    { Qt::WhatsThisPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
    { Qt::FontRole, QMetaType::QFont, nullptr, "font" },
    { Qt::TextAlignmentRole, 0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
    { Qt::BackgroundRole, QMetaType::QBrush, nullptr, "background" },
    { Qt::ForegroundRole, QMetaType::QBrush, nullptr, "foreground" },
    { ItemFlagsShadowRole, 0, QtVariantPropertyManager::flagTypeId, "flags" },
    { Qt::CheckStateRole, 0, QtVariantPropertyManager::enumTypeId, "checkState" },
    { 0, 0, nullptr, nullptr }
};

ListContents ListWidgetEditor::fillContentsFromListWidget(QListWidget *listWidget)
{
    setWindowTitle(tr("Edit List Widget"));

    ListContents retVal;
    retVal.createFromListWidget(listWidget, false);
    retVal.applyToListWidget(m_itemsEditor->listWidget(), m_itemsEditor->iconCache(), true);

    m_itemsEditor->setupEditor(listWidget, listBoxPropList);

    return retVal;
}

static AbstractItemEditor::PropertyDefinition comboBoxPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { 0, 0, nullptr, nullptr }
};

ListContents ListWidgetEditor::fillContentsFromComboBox(QComboBox *comboBox)
{
    setWindowTitle(tr("Edit Combobox"));

    ListContents retVal;
    retVal.createFromComboBox(comboBox);
    retVal.applyToListWidget(m_itemsEditor->listWidget(), m_itemsEditor->iconCache(), true);

    m_itemsEditor->setupEditor(comboBox, comboBoxPropList);

    return retVal;
}

ListContents ListWidgetEditor::contents() const
{
    ListContents retVal;
    retVal.createFromListWidget(m_itemsEditor->listWidget(), true);
    return retVal;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
