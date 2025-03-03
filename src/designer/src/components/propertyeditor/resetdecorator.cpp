// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "resetdecorator.h"
#include "qtpropertybrowser_p.h"

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

ResetWidget::ResetWidget(QtProperty *property, QWidget *parent) :
      QWidget(parent),
      m_property(property),
      m_textLabel(new QLabel(this)),
      m_iconLabel(new QLabel(this)),
      m_button(new QToolButton(this))
{
    m_textLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
    m_iconLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_button->setIcon(createIconSet("resetproperty.png"_L1));
    m_button->setIconSize(QSize(8,8));
    m_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
    connect(m_button, &QAbstractButton::clicked, this, &ResetWidget::slotClicked);
    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(m_spacing);
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);
    layout->addWidget(m_button);
    setFocusProxy(m_textLabel);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

void ResetWidget::setSpacing(int spacing)
{
    m_spacing = spacing;
    layout()->setSpacing(m_spacing);
}

void ResetWidget::setWidget(QWidget *widget)
{
    if (m_textLabel) {
        delete m_textLabel;
        m_textLabel = nullptr;
    }
    if (m_iconLabel) {
        delete m_iconLabel;
        m_iconLabel = nullptr;
    }
    delete layout();
    QLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(m_spacing);
    layout->addWidget(widget);
    layout->addWidget(m_button);
    setFocusProxy(widget);
}

void ResetWidget::setResetEnabled(bool enabled)
{
    m_button->setEnabled(enabled);
}

void ResetWidget::setValueText(const QString &text)
{
    if (m_textLabel)
        m_textLabel->setText(text);
}

void ResetWidget::setValueIcon(const QIcon &icon)
{
    QPixmap pix = icon.pixmap(QSize(16, 16));
    if (m_iconLabel) {
        m_iconLabel->setVisible(!pix.isNull());
        m_iconLabel->setPixmap(pix);
    }
}

void ResetWidget::slotClicked()
{
    emit resetProperty(m_property);
}

ResetDecorator::ResetDecorator(const QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent)
    , m_core(core)
{
}

ResetDecorator::~ResetDecorator()
{
    const auto editors = m_resetWidgetToProperty.keys();
    qDeleteAll(editors);
}

void ResetDecorator::connectPropertyManager(QtAbstractPropertyManager *manager)
{
    connect(manager, &QtAbstractPropertyManager::propertyChanged,
            this, &ResetDecorator::slotPropertyChanged);
}

void ResetDecorator::disconnectPropertyManager(QtAbstractPropertyManager *manager)
{
    disconnect(manager, &QtAbstractPropertyManager::propertyChanged,
            this, &ResetDecorator::slotPropertyChanged);
}

void ResetDecorator::setSpacing(int spacing)
{
    m_spacing = spacing;
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

QWidget *ResetDecorator::editor(QWidget *subEditor, bool resettable, QtAbstractPropertyManager *manager, QtProperty *property,
            QWidget *parent)
{
    Q_UNUSED(manager);

    ResetWidget *resetWidget = nullptr;
    if (resettable) {
        resetWidget = new ResetWidget(property, parent);
        resetWidget->setSpacing(m_spacing);
        resetWidget->setResetEnabled(property->isModified() || isModifiedInMultiSelection(m_core, property->propertyName()));
        resetWidget->setValueText(property->valueText());
        resetWidget->setValueIcon(property->valueIcon());
        resetWidget->setAutoFillBackground(true);
        connect(resetWidget, &QObject::destroyed, this, &ResetDecorator::slotEditorDestroyed);
        connect(resetWidget, &ResetWidget::resetProperty, this, &ResetDecorator::resetProperty);
        m_createdResetWidgets[property].append(resetWidget);
        m_resetWidgetToProperty[resetWidget] = property;
    }
    if (subEditor) {
        if (resetWidget) {
            subEditor->setParent(resetWidget);
            resetWidget->setWidget(subEditor);
        }
    }
    if (resetWidget)
        return resetWidget;
    return subEditor;
}

void ResetDecorator::slotPropertyChanged(QtProperty *property)
{
    const auto prIt = m_createdResetWidgets.constFind(property);
    if (prIt == m_createdResetWidgets.constEnd())
        return;

    for (ResetWidget *widget : prIt.value()) {
        widget->setResetEnabled(property->isModified()
                                || isModifiedInMultiSelection(m_core, property->propertyName()));
        widget->setValueText(property->valueText());
        widget->setValueIcon(property->valueIcon());
    }
}

void ResetDecorator::slotEditorDestroyed(QObject *object)
{
    for (auto itEditor = m_resetWidgetToProperty.cbegin(), cend = m_resetWidgetToProperty.cend(); itEditor != cend; ++itEditor) {
        if (itEditor.key() == object) {
            ResetWidget *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            m_resetWidgetToProperty.remove(editor);
            m_createdResetWidgets[property].removeAll(editor);
            if (m_createdResetWidgets[property].isEmpty())
                m_createdResetWidgets.remove(property);
            return;
        }
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
