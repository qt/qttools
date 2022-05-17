// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "newdynamicpropertydialog.h"
#include "ui_newdynamicpropertydialog.h"
#include <abstractdialoggui_p.h>
#include <qdesigner_propertysheet_p.h>

#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

NewDynamicPropertyDialog::NewDynamicPropertyDialog(QDesignerDialogGuiInterface *dialogGui,
                                                       QWidget *parent)   :
    QDialog(parent),
    m_dialogGui(dialogGui),
    m_ui(new Ui::NewDynamicPropertyDialog)
{
    m_ui->setupUi(this);
    connect(m_ui->m_lineEdit, &QLineEdit::textChanged, this, &NewDynamicPropertyDialog::nameChanged);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_ui->m_comboBox->addItem(QStringLiteral("String"),
                              QVariant(QMetaType(QMetaType::QString)));
    m_ui->m_comboBox->addItem(QStringLiteral("StringList"),
                              QVariant(QMetaType(QMetaType::QStringList)));
    m_ui->m_comboBox->addItem(QStringLiteral("Char"),
                              QVariant(QMetaType(QMetaType::QChar)));
    m_ui->m_comboBox->addItem(QStringLiteral("ByteArray"),
                              QVariant(QMetaType(QMetaType::QByteArray)));
    m_ui->m_comboBox->addItem(QStringLiteral("Url"),
                              QVariant(QMetaType(QMetaType::QUrl)));
    m_ui->m_comboBox->addItem(QStringLiteral("Bool"),
                              QVariant(QMetaType(QMetaType::Bool)));
    m_ui->m_comboBox->addItem(QStringLiteral("Int"),
                              QVariant(QMetaType(QMetaType::Int)));
    m_ui->m_comboBox->addItem(QStringLiteral("UInt"),
                              QVariant(QMetaType(QMetaType::UInt)));
    m_ui->m_comboBox->addItem(QStringLiteral("LongLong"),
                              QVariant(QMetaType(QMetaType::LongLong)));
    m_ui->m_comboBox->addItem(QStringLiteral("ULongLong"),
                              QVariant(QMetaType(QMetaType::ULongLong)));
    m_ui->m_comboBox->addItem(QStringLiteral("Double"),
                              QVariant(QMetaType(QMetaType::Double)));
    m_ui->m_comboBox->addItem(QStringLiteral("Size"),
                              QVariant(QMetaType(QMetaType::QSize)));
    m_ui->m_comboBox->addItem(QStringLiteral("SizeF"),
                              QVariant(QMetaType(QMetaType::QSizeF)));
    m_ui->m_comboBox->addItem(QStringLiteral("Point"),
                              QVariant(QMetaType(QMetaType::QPoint)));
    m_ui->m_comboBox->addItem(QStringLiteral("PointF"),
                              QVariant(QMetaType(QMetaType::QPointF)));
    m_ui->m_comboBox->addItem(QStringLiteral("Rect"),
                              QVariant(QMetaType(QMetaType::QRect)));
    m_ui->m_comboBox->addItem(QStringLiteral("RectF"),
                              QVariant(QMetaType(QMetaType::QRectF)));
    m_ui->m_comboBox->addItem(QStringLiteral("Date"),
                              QVariant(QMetaType(QMetaType::QDate)));
    m_ui->m_comboBox->addItem(QStringLiteral("Time"),
                              QVariant(QMetaType(QMetaType::QTime)));
    m_ui->m_comboBox->addItem(QStringLiteral("DateTime"),
                              QVariant(QMetaType(QMetaType::QDateTime)));
    m_ui->m_comboBox->addItem(QStringLiteral("Font"),
                              QVariant(QMetaType(QMetaType::QFont)));
    m_ui->m_comboBox->addItem(QStringLiteral("Palette"),
                              QVariant(QMetaType(QMetaType::QPalette)));
    m_ui->m_comboBox->addItem(QStringLiteral("Color"),
                              QVariant(QMetaType(QMetaType::QColor)));
    m_ui->m_comboBox->addItem(QStringLiteral("Pixmap"),
                              QVariant(QMetaType(QMetaType::QPixmap)));
    m_ui->m_comboBox->addItem(QStringLiteral("Icon"),
                              QVariant(QMetaType(QMetaType::QIcon)));
    m_ui->m_comboBox->addItem(QStringLiteral("Cursor"),
                              QVariant(QMetaType(QMetaType::QCursor)));
    m_ui->m_comboBox->addItem(QStringLiteral("SizePolicy"),
                              QVariant(QMetaType(QMetaType::QSizePolicy)));
    m_ui->m_comboBox->addItem(QStringLiteral("KeySequence"),
                              QVariant(QMetaType(QMetaType::QKeySequence)));

    m_ui->m_comboBox->setCurrentIndex(0); // String
    setOkButtonEnabled(false);
}

void NewDynamicPropertyDialog::setOkButtonEnabled(bool e)
{
    m_ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(e);
}

NewDynamicPropertyDialog::~NewDynamicPropertyDialog()
{
    delete m_ui;
}

void NewDynamicPropertyDialog::setReservedNames(const QStringList &names)
{
    m_reservedNames = names;
}

void NewDynamicPropertyDialog::setPropertyType(int t)
{
    const int index = m_ui->m_comboBox->findData(QVariant(QMetaType(t)));
    if (index != -1)
        m_ui->m_comboBox->setCurrentIndex(index);
}

QString NewDynamicPropertyDialog::propertyName() const
{
    return m_ui->m_lineEdit->text();
}

QVariant NewDynamicPropertyDialog::propertyValue() const
{
    const int index = m_ui->m_comboBox->currentIndex();
    if (index == -1)
        return QVariant();
    return m_ui->m_comboBox->itemData(index);
}

void NewDynamicPropertyDialog::information(const QString &message)
{
    m_dialogGui->message(this, QDesignerDialogGuiInterface::PropertyEditorMessage, QMessageBox::Information, tr("Set Property Name"), message);
}

void NewDynamicPropertyDialog::nameChanged(const QString &s)
{
    setOkButtonEnabled(!s.isEmpty());
}

bool NewDynamicPropertyDialog::validatePropertyName(const QString& name)
{
    if (m_reservedNames.contains(name)) {
        information(tr("The current object already has a property named '%1'.\nPlease select another, unique one.").arg(name));
        return false;
    }
    if (!QDesignerPropertySheet::internalDynamicPropertiesEnabled() && name.startsWith(QStringLiteral("_q_"))) {
        information(tr("The '_q_' prefix is reserved for the Qt library.\nPlease select another name."));
        return false;
    }
    return true;
}

void NewDynamicPropertyDialog::on_m_buttonBox_clicked(QAbstractButton *btn)
{
    const int role = m_ui->m_buttonBox->buttonRole(btn);
    switch (role) {
        case QDialogButtonBox::RejectRole:
            reject();
            break;
        case QDialogButtonBox::AcceptRole:
            if (validatePropertyName(propertyName()))
                accept();
            break;
    }
}
}

QT_END_NAMESPACE
