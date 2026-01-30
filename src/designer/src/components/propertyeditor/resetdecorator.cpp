// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "resetdecorator.h"
#include "qtpropertybrowser_p.h"
#include "qtpropertybrowserutils_p.h"

#include <qdesigner_utils_p.h>
#include <iconloader_p.h>
#include <formwindowbase_p.h>
#include <formwindowcursor.h>
#include <formwindowmanager.h>
#include <formwindow.h>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/propertysheet.h>

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qtoolbutton.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

using namespace Qt::StringLiterals;

ResetWidget::ResetWidget(QWidget *editor, QWidget *parent)
    : QWidget(parent), m_button(new QToolButton(this))
{
    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->addWidget(editor);

    m_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_button->setIcon(createIconSet("resetproperty.png"_L1));
    m_button->setIconSize(QSize(8,8));
    m_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
    connect(m_button, &QAbstractButton::clicked, this, &ResetWidget::reset);
    layout->addWidget(m_button);
    setFocusProxy(editor);
    setAutoFillBackground(true); // Hide value icon/text
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

void ResetWidget::setSpacing(int spacing)
{
    layout()->setSpacing(spacing);
}

void ResetWidget::setResetEnabled(bool enabled)
{
    m_button->setEnabled(enabled);
}

PropertyResetWidget::PropertyResetWidget(const QDesignerFormEditorInterface *core,
                                         QtProperty *property, QWidget *editor, QWidget *parent)
    : ResetWidget(editor, parent), m_core(core), m_property(property)
{
    connect(this, &ResetWidget::reset, this, &PropertyResetWidget::emitResetProperty);
}

void PropertyResetWidget::emitResetProperty()
{
    emit resetProperty(m_property);
}

static inline bool isModifiedInMultiSelection(const QDesignerFormEditorInterface *core,
                                              const QString &propertyName)
{
    const QDesignerFormWindowInterface *form = core->formWindowManager()->activeFormWindow();
    if (!form)
        return false;
    const QDesignerFormWindowCursorInterface *cursor = form->cursor();
    const int selectionSize = cursor->selectedWidgetCount();
    if (selectionSize < 2)
        return false;
    for (int i = 0; i < selectionSize; ++i) {
        const QDesignerPropertySheetExtension *sheet =
            qt_extension<QDesignerPropertySheetExtension*>(core->extensionManager(),
                                                           cursor->selectedWidget(i));
        const int index = sheet->indexOf(propertyName);
        if (index >= 0 && sheet->isChanged(index))
            return true;
    }
    return false;
}

void PropertyResetWidget::propertyChanged(QtProperty *property)
{
    if (property == m_property) {
        // Update the resettable state as the user edits or resets.
        const bool resettable = m_property->isModified()
                || isModifiedInMultiSelection(m_core, m_property->propertyName());
        setResetEnabled(resettable);
    }
}

DummyEditor::DummyEditor(QtProperty *property, QWidget *parent)
    : QWidget(parent),
      m_property(property),
      m_textLabel(new QLabel(this)),
      m_iconLabel(new QLabel(this))
{
    m_textLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
    m_iconLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);
}

void DummyEditor::setSpacing(int spacing)
{
    layout()->setSpacing(spacing);
}

void DummyEditor::setValueIcon(const QIcon &icon)
{
    QPixmap pix = icon.pixmap(QtPropertyBrowserUtils::itemViewIconSize, devicePixelRatioF());
    m_iconLabel->setVisible(!pix.isNull());
    m_iconLabel->setPixmap(pix);
}

void DummyEditor::propertyChanged(QtProperty *property)
{
    if (m_property == property) {
        m_textLabel->setText(property->valueText());
        setValueIcon(property->valueIcon());
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
