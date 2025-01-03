// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "deviceprofiledialog.h"
#include "ui_deviceprofiledialog.h"

#include <abstractdialoggui_p.h>
#include <deviceprofile_p.h>

#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qstylefactory.h>
#include <QtGui/qfontdatabase.h>
#include <QtGui/qvalidator.h>

#include <QtCore/qfileinfo.h>
#include <QtCore/qfile.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static const char profileExtensionC[] = "qdp";

static inline QString fileFilter()
{
    return qdesigner_internal::DeviceProfileDialog::tr("Device Profiles (*.%1)").arg(QLatin1StringView(profileExtensionC));
}

// Populate a combo with a sequence of integers, also set them as data.
template <class IntIterator>
    static void populateNumericCombo(IntIterator i1, IntIterator i2, QComboBox *cb)
{
    QString s;
    for ( ; i1 != i2 ; ++i1) {
        const int n = *i1;
        s.setNum(n);
        cb->addItem(s, QVariant(n));
    }
}

namespace qdesigner_internal {

DeviceProfileDialog::DeviceProfileDialog(QDesignerDialogGuiInterface *dlgGui, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DeviceProfileDialog),
    m_dlgGui(dlgGui)
{
    setModal(true);
    m_ui->setupUi(this);

    const auto standardFontSizes = QFontDatabase::standardSizes();
    populateNumericCombo(standardFontSizes.constBegin(), standardFontSizes.constEnd(), m_ui->m_systemFontSizeCombo);

    // 288pt observed on macOS.
    const int maxPointSize = qMax(288, standardFontSizes.constLast());
    m_ui->m_systemFontSizeCombo->setValidator(new QIntValidator(1, maxPointSize,
                                                                m_ui->m_systemFontSizeCombo));

    // Styles
    const QStringList styles = QStyleFactory::keys();
    m_ui->m_styleCombo->addItem(tr("Default"), QVariant(QString()));
    for (const auto &s : styles)
         m_ui->m_styleCombo->addItem(s, s);

    connect(m_ui->m_nameLineEdit, &QLineEdit::textChanged, this, &DeviceProfileDialog::nameChanged);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked,
            this, &QDialog::accept);
    // Note that Load/Save emit accepted() of the button box..
    connect(m_ui->buttonBox->button(QDialogButtonBox::Save), &QAbstractButton::clicked,
            this, &DeviceProfileDialog::save);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Open), &QAbstractButton::clicked,
            this, &DeviceProfileDialog::open);
}

DeviceProfileDialog::~DeviceProfileDialog()
{
    delete m_ui;
}

DeviceProfile DeviceProfileDialog::deviceProfile() const
{
    DeviceProfile rc;
    rc.setName(m_ui->m_nameLineEdit->text());
    rc.setFontFamily(m_ui->m_systemFontComboBox->currentFont().family());
    rc.setFontPointSize(m_ui->m_systemFontSizeCombo->itemData(m_ui->m_systemFontSizeCombo->currentIndex()).toInt());

    int dpiX, dpiY;
    m_ui->m_dpiChooser->getDPI(&dpiX, &dpiY);
    rc.setDpiX(dpiX);
    rc.setDpiY(dpiY);

    rc.setStyle(m_ui->m_styleCombo->itemData(m_ui->m_styleCombo->currentIndex()).toString());

    return rc;
}

void DeviceProfileDialog::setDeviceProfile(const DeviceProfile &s)
{
    m_ui->m_nameLineEdit->setText(s.name());
    m_ui->m_systemFontComboBox->setCurrentFont(QFont(s.fontFamily()));
    const int fontSizeIndex = m_ui->m_systemFontSizeCombo->findData(QVariant(s.fontPointSize()));
    m_ui->m_systemFontSizeCombo->setCurrentIndex(fontSizeIndex != -1 ? fontSizeIndex : 0);
    m_ui->m_dpiChooser->setDPI(s.dpiX(), s.dpiY());
    const int styleIndex = m_ui->m_styleCombo->findData(s.style());
    m_ui->m_styleCombo->setCurrentIndex(styleIndex != -1 ? styleIndex : 0);
}

void DeviceProfileDialog::setOkButtonEnabled(bool v)
{
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(v);
}

bool DeviceProfileDialog::showDialog(const QStringList &existingNames)
{
    m_existingNames = existingNames;
    m_ui->m_nameLineEdit->setFocus(Qt::OtherFocusReason);
    nameChanged(m_ui->m_nameLineEdit->text());
    return exec() == Accepted;
}

void DeviceProfileDialog::nameChanged(const QString &name)
{
    const bool invalid = name.isEmpty() || m_existingNames.indexOf(name) != -1;
    setOkButtonEnabled(!invalid);
}

void DeviceProfileDialog::save()
{
    QString fn = m_dlgGui->getSaveFileName(this, tr("Save Profile"), QString(), fileFilter());
    if (fn.isEmpty())
        return;
    if (QFileInfo(fn).completeSuffix().isEmpty())
        fn += u'.' + QLatin1StringView(profileExtensionC);

    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        critical(tr("Save Profile - Error"), tr("Unable to open the file '%1' for writing: %2").arg(fn, file.errorString()));
        return;
    }
    file.write(deviceProfile().toXml().toUtf8());
}

void DeviceProfileDialog::open()
{
    const QString fn = m_dlgGui->getOpenFileName(this, tr("Open profile"), QString(), fileFilter());
    if (fn.isEmpty())
        return;

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        critical(tr("Open Profile - Error"), tr("Unable to open the file '%1' for reading: %2").arg(fn, file.errorString()));
        return;
    }
    QString errorMessage;
    DeviceProfile newSettings;
    if (!newSettings.fromXml(QString::fromUtf8(file.readAll()), &errorMessage)) {
        critical(tr("Open Profile - Error"), tr("'%1' is not a valid profile: %2").arg(fn, errorMessage));
        return;
    }
    setDeviceProfile(newSettings);
}

void DeviceProfileDialog::critical(const QString &title, const QString &msg)
{
    m_dlgGui->message(this, QDesignerDialogGuiInterface::OtherMessage, QMessageBox::Critical, title, msg);
}
}

QT_END_NAMESPACE
