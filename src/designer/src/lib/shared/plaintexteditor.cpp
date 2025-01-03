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

static const char PlainTextDialogC[] = "PlainTextDialog";
static const char PlainTextEditorGeometryC[] = "Geometry";


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
    settings->beginGroup(QLatin1StringView(PlainTextDialogC));

    if (settings->contains(QLatin1StringView(PlainTextEditorGeometryC)))
        restoreGeometry(settings->value(QLatin1StringView(PlainTextEditorGeometryC)).toByteArray());

    settings->endGroup();
}

PlainTextEditorDialog::~PlainTextEditorDialog()
{
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(QLatin1StringView(PlainTextDialogC));

    settings->setValue(QLatin1StringView(PlainTextEditorGeometryC), saveGeometry());
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
