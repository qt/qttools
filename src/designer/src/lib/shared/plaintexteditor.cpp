// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "plaintexteditor_p.h"

#include <QtDesigner/abstractsettings.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto PlainTextDialogC = "PlainTextDialog"_L1;
static constexpr auto PlainTextEditorGeometryC = "Geometry"_L1;

namespace qdesigner_internal {

PlainTextEditorDialog::PlainTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent)  :
    QDialog(parent),
    m_editor(new QPlainTextEdit),
    m_core(core)
{
    setWindowTitle(tr("Edit text"));

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addWidget(m_editor);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
    ok_button->setDefault(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    vlayout->addWidget(buttonBox);

    QDesignerSettingsInterface *settings = core->settingsManager();
    settings->beginGroup(PlainTextDialogC);

    if (settings->contains(PlainTextEditorGeometryC))
        restoreGeometry(settings->value(PlainTextEditorGeometryC).toByteArray());

    settings->endGroup();
}

PlainTextEditorDialog::~PlainTextEditorDialog()
{
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(PlainTextDialogC);

    settings->setValue(PlainTextEditorGeometryC, saveGeometry());
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
