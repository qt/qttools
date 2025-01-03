// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "connectdialog_p.h"
#include "signalslot_utils_p.h"

#include <signalslotdialog_p.h>
#include <metadatabase_p.h>

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractlanguage.h>

#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static QString realClassName(QDesignerFormEditorInterface *core, QWidget *widget)
{
    QString class_name = QLatin1StringView(widget->metaObject()->className());
    const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
    const int idx = wdb->indexOfObject(widget);
    if (idx != -1)
        class_name = wdb->item(idx)->name();
    return class_name;
}

static QString widgetLabel(QDesignerFormEditorInterface *core, QWidget *widget)
{
    return "%1 (%2)"_L1
            .arg(qdesigner_internal::realObjectName(core, widget),
                 realClassName(core, widget));
}

namespace qdesigner_internal {

ConnectDialog::ConnectDialog(QDesignerFormWindowInterface *formWindow,
                             QWidget *source, QWidget *destination,
                             QWidget *parent) :
    QDialog(parent),
    m_source(source),
    m_destination(destination),
    m_sourceMode(widgetMode(m_source, formWindow)),
    m_destinationMode(widgetMode(m_destination, formWindow)),
    m_formWindow(formWindow)
{
    m_ui.setupUi(this);

    connect(m_ui.signalList, &QListWidget::itemClicked,
            this, &ConnectDialog::selectSignal);
    connect(m_ui.slotList, &QListWidget::itemClicked,
            this, &ConnectDialog::selectSlot);
    m_ui.slotList->setEnabled(false);

    QPushButton *ok_button = okButton();
    ok_button->setDefault(true);
    ok_button->setEnabled(false);

    connect(m_ui.showAllCheckBox, &QCheckBox::toggled, this, &ConnectDialog::populateLists);

    QDesignerFormEditorInterface *core = m_formWindow->core();
    m_ui.signalGroupBox->setTitle(widgetLabel(core, source));
    m_ui.slotGroupBox->setTitle(widgetLabel(core, destination));

    m_ui.editSignalsButton->setEnabled(m_sourceMode != NormalWidget);
    connect(m_ui.editSignalsButton, &QAbstractButton::clicked,
            this, &ConnectDialog::editSignals);

    m_ui.editSlotsButton->setEnabled(m_destinationMode != NormalWidget);
    connect(m_ui.editSlotsButton, &QAbstractButton::clicked,
            this, &ConnectDialog::editSlots);

    populateLists();
}

ConnectDialog::WidgetMode ConnectDialog::widgetMode(QWidget *w,  QDesignerFormWindowInterface *formWindow)
{
    QDesignerFormEditorInterface *core = formWindow->core();
    if (qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core))
        return NormalWidget;

    if (w == formWindow || formWindow->mainContainer() == w)
        return MainContainer;

    if (isPromoted(formWindow->core(), w))
        return PromotedWidget;

    return NormalWidget;
}

QPushButton *ConnectDialog::okButton()
{
    return m_ui.buttonBox->button(QDialogButtonBox::Ok);
}

void ConnectDialog::setOkButtonEnabled(bool e)
{
    okButton()->setEnabled(e);
}

void ConnectDialog::populateLists()
{
    populateSignalList();
}

void ConnectDialog::setSignalSlot(const QString &signal, const QString &slot)
{
    auto sigItems = m_ui.signalList->findItems(signal, Qt::MatchExactly);

    if (sigItems.isEmpty()) {
        m_ui.showAllCheckBox->setChecked(true);
        sigItems = m_ui.signalList->findItems(signal, Qt::MatchExactly);
    }

    if (!sigItems.isEmpty()) {
        selectSignal(sigItems.constFirst());
        auto slotItems = m_ui.slotList->findItems(slot, Qt::MatchExactly);
        if (slotItems.isEmpty()) {
            m_ui.showAllCheckBox->setChecked(true);
            slotItems = m_ui.slotList->findItems(slot, Qt::MatchExactly);
        }
        if (!slotItems.isEmpty())
            selectSlot(slotItems.constFirst());
    }
}

bool ConnectDialog::showAllSignalsSlots() const
{
    return m_ui.showAllCheckBox->isChecked();
}

void ConnectDialog::setShowAllSignalsSlots(bool showIt)
{
    m_ui.showAllCheckBox->setChecked(showIt);
}

void ConnectDialog::selectSignal(QListWidgetItem *item)
{
    if (item) {
        m_ui.signalList->setCurrentItem(item);
        populateSlotList(item->text());
        m_ui.slotList->setEnabled(true);
        setOkButtonEnabled(!m_ui.slotList->selectedItems().isEmpty());
    } else {
        m_ui.signalList->clearSelection();
        populateSlotList();
        m_ui.slotList->setEnabled(false);
        setOkButtonEnabled(false);
    }
}

void ConnectDialog::selectSlot(QListWidgetItem *item)
{
    if (item) {
        m_ui.slotList->setCurrentItem(item);
    } else {
        m_ui.slotList->clearSelection();
    }
    setOkButtonEnabled(true);
}

QString ConnectDialog::signal() const
{
    const auto item_list = m_ui.signalList->selectedItems();
    if (item_list.size() != 1)
        return QString();
    return item_list.at(0)->text();
}

QString ConnectDialog::slot() const
{
    const auto item_list = m_ui.slotList->selectedItems();
    if (item_list.size() != 1)
        return QString();
    return item_list.at(0)->text();
}

void ConnectDialog::populateSlotList(const QString &signal)
{
    enum { deprecatedSlot = 0 };
    QString selectedName;
    if (const QListWidgetItem * item = m_ui.slotList->currentItem())
        selectedName = item->text();

    m_ui.slotList->clear();

    QMap<QString, QString> memberToClassName = getMatchingSlots(m_formWindow->core(), m_destination, signal, showAllSignalsSlots());

    QFont font = QApplication::font();
    font.setItalic(true);
    QVariant variantFont = QVariant::fromValue(font);

    QListWidgetItem *curr = nullptr;
    for (auto itMember = memberToClassName.cbegin(), itMemberEnd = memberToClassName.cend(); itMember != itMemberEnd; ++itMember) {
        const QString member = itMember.key();
        QListWidgetItem *item = new QListWidgetItem(m_ui.slotList);
        item->setText(member);
        if (member == selectedName)
            curr = item;

        // Mark deprecated slots red. Not currently in use (historically for Qt 3 slots in Qt 4),
        // but may be used again in the future.
        if (deprecatedSlot) {
            item->setData(Qt::FontRole, variantFont);
            item->setData(Qt::ForegroundRole, QColor(Qt::red));
        }
    }

    if (curr)
        m_ui.slotList->setCurrentItem(curr);

    if (m_ui.slotList->selectedItems().isEmpty())
        setOkButtonEnabled(false);
}

void ConnectDialog::populateSignalList()
{
    enum { deprecatedSignal = 0 };

    QString selectedName;
    if (const QListWidgetItem *item = m_ui.signalList->currentItem())
        selectedName = item->text();

    m_ui.signalList->clear();

    QMap<QString, QString> memberToClassName = getSignals(m_formWindow->core(), m_source, showAllSignalsSlots());

    QFont font = QApplication::font();
    font.setItalic(true);
    QVariant variantFont = QVariant::fromValue(font);

    QListWidgetItem *curr = nullptr;
    for (auto itMember = memberToClassName.cbegin(), itMemberEnd = memberToClassName.cend(); itMember != itMemberEnd; ++itMember) {
        const QString member = itMember.key();

        QListWidgetItem *item = new QListWidgetItem(m_ui.signalList);
        item->setText(member);
        if (!selectedName.isEmpty() && member == selectedName)
            curr = item;

        // Mark deprecated signals red. Not currently in use (historically for Qt 3 slots in Qt 4),
        // but may be used again in the future.
        if (deprecatedSignal) {
            item->setData(Qt::FontRole, variantFont);
            item->setData(Qt::ForegroundRole, QColor(Qt::red));
        }
    }

    if (curr) {
        m_ui.signalList->setCurrentItem(curr);
    } else {
        selectedName.clear();
    }

    populateSlotList(selectedName);
    if (!curr)
        m_ui.slotList->setEnabled(false);
}

void ConnectDialog::editSignals()
{
    editSignalsSlots(m_source, m_sourceMode, SignalSlotDialog::FocusSignals);
}

void ConnectDialog::editSlots()
{
    editSignalsSlots(m_destination, m_destinationMode, SignalSlotDialog::FocusSlots);
}

void ConnectDialog::editSignalsSlots(QWidget *w, WidgetMode mode, int signalSlotDialogModeInt)
{
    const SignalSlotDialog::FocusMode signalSlotDialogMode = static_cast<SignalSlotDialog::FocusMode>(signalSlotDialogModeInt);
    switch (mode) {
    case  NormalWidget:
        break;
    case MainContainer:
        if (SignalSlotDialog::editMetaDataBase(m_formWindow, w, this, signalSlotDialogMode))
            populateLists();
        break;
    case PromotedWidget:
        if (SignalSlotDialog::editPromotedClass(m_formWindow->core(), w, this, signalSlotDialogMode))
            populateLists();
        break;
    }
}

}

QT_END_NAMESPACE

#include "moc_connectdialog_p.cpp"

