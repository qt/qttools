// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qtgradientviewdialog.h"
#include "qtgradientmanager.h"
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

QtGradientViewDialog::QtGradientViewDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(m_ui.gradientView, &QtGradientView::currentGradientChanged,
            this, &QtGradientViewDialog::slotGradientSelected);
    connect(m_ui.gradientView, &QtGradientView::gradientActivated,
            this, &QtGradientViewDialog::slotGradientActivated);
}

void QtGradientViewDialog::setGradientManager(QtGradientManager *manager)
{
    m_ui.gradientView->setGradientManager(manager);
}

QGradient QtGradientViewDialog::getGradient(bool *ok, QtGradientManager *manager, QWidget *parent, const QString &caption)
{
    QtGradientViewDialog dlg(parent);
    dlg.setGradientManager(manager);
    dlg.setWindowTitle(caption);
    QGradient grad = QLinearGradient();
    const int res = dlg.exec();
    if (res == QDialog::Accepted)
        grad = dlg.m_ui.gradientView->gradientManager()->gradients().value(dlg.m_ui.gradientView->currentGradient());
    if (ok)
        *ok = res == QDialog::Accepted;
    return grad;
}

void QtGradientViewDialog::slotGradientSelected(const QString &id)
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!id.isEmpty());
}

void QtGradientViewDialog::slotGradientActivated(const QString &id)
{
    Q_UNUSED(id);
    accept();
}

QT_END_NAMESPACE
