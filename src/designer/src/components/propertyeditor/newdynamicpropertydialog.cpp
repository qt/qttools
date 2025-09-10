// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "newdynamicpropertydialog.h"
#include "ui_newdynamicpropertydialog.h"
#include <abstractdialoggui_p.h>
#include <qdesigner_propertysheet_p.h>

#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

NewDynamicPropertyDialog::NewDynamicPropertyDialog(QDesignerDialogGuiInterface *dialogGui,
                                                       QWidget *parent)   :
    QDialog(parent),
    m_dialogGui(dialogGui),
    m_ui(new QT_PREPEND_NAMESPACE(qdesigner_internal)::Ui::NewDynamicPropertyDialog)
{
    m_ui->setupUi(this);
    connect(m_ui->m_lineEdit, &QLineEdit::textChanged, this, &NewDynamicPropertyDialog::nameChanged);
    connect(m_ui->m_buttonBox, &QDialogButtonBox::clicked,
            this, &NewDynamicPropertyDialog::buttonBoxClicked);

    m_ui->m_comboBox->addItem(u"String"_s,
                              QVariant(QMetaType(QMetaType::QString)));
    m_ui->m_comboBox->addItem(u"StringList"_s,
                              QVariant(QMetaType(QMetaType::QStringList)));
    m_ui->m_comboBox->addItem(u"Char"_s,
                              QVariant(QMetaType(QMetaType::QChar)));
    m_ui->m_comboBox->addItem(u"ByteArray"_s,
                              QVariant(QMetaType(QMetaType::QByteArray)));
    m_ui->m_comboBox->addItem(u"Url"_s,
                              QVariant(QMetaType(QMetaType::QUrl)));
    m_ui->m_comboBox->addItem(u"Bool"_s,
                              QVariant(QMetaType(QMetaType::Bool)));
    m_ui->m_comboBox->addItem(u"Int"_s,
                              QVariant(QMetaType(QMetaType::Int)));
    m_ui->m_comboBox->addItem(u"UInt"_s,
                              QVariant(QMetaType(QMetaType::UInt)));
    m_ui->m_comboBox->addItem(u"LongLong"_s,
                              QVariant(QMetaType(QMetaType::LongLong)));
    m_ui->m_comboBox->addItem(u"ULongLong"_s,
                              QVariant(QMetaType(QMetaType::ULongLong)));
    m_ui->m_comboBox->addItem(u"Double"_s,
                              QVariant(QMetaType(QMetaType::Double)));
    m_ui->m_comboBox->addItem(u"Size"_s,
                              QVariant(QMetaType(QMetaType::QSize)));
    m_ui->m_comboBox->addItem(u"SizeF"_s,
                              QVariant(QMetaType(QMetaType::QSizeF)));
    m_ui->m_comboBox->addItem(u"Point"_s,
                              QVariant(QMetaType(QMetaType::QPoint)));
    m_ui->m_comboBox->addItem(u"PointF"_s,
                              QVariant(QMetaType(QMetaType::QPointF)));
    m_ui->m_comboBox->addItem(u"Rect"_s,
                              QVariant(QMetaType(QMetaType::QRect)));
    m_ui->m_comboBox->addItem(u"RectF"_s,
                              QVariant(QMetaType(QMetaType::QRectF)));
    m_ui->m_comboBox->addItem(u"Date"_s,
                              QVariant(QMetaType(QMetaType::QDate)));
    m_ui->m_comboBox->addItem(u"Time"_s,
                              QVariant(QMetaType(QMetaType::QTime)));
    m_ui->m_comboBox->addItem(u"DateTime"_s,
                              QVariant(QMetaType(QMetaType::QDateTime)));
    m_ui->m_comboBox->addItem(u"Font"_s,
                              QVariant(QMetaType(QMetaType::QFont)));
    m_ui->m_comboBox->addItem(u"Palette"_s,
                              QVariant(QMetaType(QMetaType::QPalette)));
    m_ui->m_comboBox->addItem(u"Color"_s,
                              QVariant(QMetaType(QMetaType::QColor)));
    m_ui->m_comboBox->addItem(u"Pixmap"_s,
                              QVariant(QMetaType(QMetaType::QPixmap)));
    m_ui->m_comboBox->addItem(u"Icon"_s,
                              QVariant(QMetaType(QMetaType::QIcon)));
    m_ui->m_comboBox->addItem(u"Cursor"_s,
                              QVariant(QMetaType(QMetaType::QCursor)));
    m_ui->m_comboBox->addItem(u"SizePolicy"_s,
                              QVariant(QMetaType(QMetaType::QSizePolicy)));
    m_ui->m_comboBox->addItem(u"KeySequence"_s,
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
    if (!QDesignerPropertySheet::internalDynamicPropertiesEnabled() && name.startsWith("_q_"_L1)) {
        information(tr("The '_q_' prefix is reserved for the Qt library.\nPlease select another name."));
        return false;
    }
    return true;
}

void NewDynamicPropertyDialog::buttonBoxClicked(QAbstractButton *btn)
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
