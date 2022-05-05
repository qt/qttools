/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#include "plaintexteditor_p.h"

#include <QtDesigner/abstractsettings.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

static const char *PlainTextDialogC = "PlainTextDialog";
static const char *Geometry = "Geometry";


namespace qdesigner_internal {

PlainTextEditorDialog::PlainTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent)  :
    QDialog(parent),
    m_editor(new QPlainTextEdit),
    m_core(core)
{
    setWindowTitle(tr("Edit text"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addWidget(m_editor);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
    ok_button->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    vlayout->addWidget(buttonBox);

    QDesignerSettingsInterface *settings = core->settingsManager();
    settings->beginGroup(QLatin1String(PlainTextDialogC));

    if (settings->contains(QLatin1String(Geometry)))
        restoreGeometry(settings->value(QLatin1String(Geometry)).toByteArray());

    settings->endGroup();
}

PlainTextEditorDialog::~PlainTextEditorDialog()
{
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(QLatin1String(PlainTextDialogC));

    settings->setValue(QLatin1String(Geometry), saveGeometry());
    settings->endGroup();
}

int PlainTextEditorDialog::showDialog()
{
    m_editor->setFocus();
    return exec();
}

void PlainTextEditorDialog::setDefaultFont(const QFont &font)
{
    m_editor->setFont(font);
}

void PlainTextEditorDialog::setText(const QString &text)
{
    m_editor->setPlainText(text);
}

QString PlainTextEditorDialog::text() const
{
    return m_editor->toPlainText();
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
