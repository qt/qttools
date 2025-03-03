// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "texteditor.h"

#include <abstractdialoggui_p.h>
#include <iconselector_p.h>
#include <stylesheeteditor_p.h>
#include <richtexteditor_p.h>
#include <plaintexteditor_p.h>

#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qtoolbutton.h>

#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal
{

TextEditor::TextEditor(QDesignerFormEditorInterface *core, QWidget *parent) :
    QWidget(parent),
    m_editor(new TextPropertyEditor(this)),
    m_themeEditor(new IconThemeEditor(this, false)),
    m_iconThemeModeEnabled(false),
    m_richTextDefaultFont(QApplication::font()),
    m_button(new QToolButton(this)),
    m_menu(new QMenu(this)),
    m_resourceAction(new QAction(tr("Choose Resource..."), this)),
    m_fileAction(new QAction(tr("Choose File..."), this)),
    m_layout(new QHBoxLayout(this)),
    m_core(core)
{
    m_themeEditor->setVisible(false);
    m_button->setVisible(false);

    m_layout->addWidget(m_editor);
    m_layout->addWidget(m_themeEditor);
    m_button->setText(tr("..."));
    m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    m_button->setFixedWidth(20);
    m_layout->addWidget(m_button);
    m_layout->setContentsMargins(QMargins());
    m_layout->setSpacing(0);

    connect(m_resourceAction, &QAction::triggered, this, &TextEditor::resourceActionActivated);
    connect(m_fileAction, &QAction::triggered, this, &TextEditor::fileActionActivated);
    connect(m_editor, &TextPropertyEditor::textChanged, this, &TextEditor::textChanged);
    connect(m_themeEditor, &IconThemeEditor::edited, this, &TextEditor::textChanged);
    connect(m_button, &QAbstractButton::clicked, this, &TextEditor::buttonClicked);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    setFocusProxy(m_editor);

    m_menu->addAction(m_resourceAction);
    m_menu->addAction(m_fileAction);
}

void TextEditor::setSpacing(int spacing)
{
    m_layout->setSpacing(spacing);
}

void TextEditor::setIconThemeModeEnabled(bool enable)
{
    if (m_iconThemeModeEnabled == enable)
        return; // nothing changes
    m_iconThemeModeEnabled = enable;
    m_editor->setVisible(!enable);
    m_themeEditor->setVisible(enable);
    if (enable) {
        m_themeEditor->setTheme(m_editor->text());
        setFocusProxy(m_themeEditor);
    } else {
        m_editor->setText(m_themeEditor->theme());
        setFocusProxy(m_editor);
    }
}

TextPropertyValidationMode TextEditor::textPropertyValidationMode() const
{
    return m_editor->textPropertyValidationMode();
}

void TextEditor::setTextPropertyValidationMode(TextPropertyValidationMode vm)
{
    m_editor->setTextPropertyValidationMode(vm);
    if (vm == ValidationURL) {
        m_button->setMenu(m_menu);
        m_button->setFixedWidth(30);
        m_button->setPopupMode(QToolButton::MenuButtonPopup);
    } else {
        m_button->setMenu(nullptr);
        m_button->setFixedWidth(20);
        m_button->setPopupMode(QToolButton::DelayedPopup);
    }
    m_button->setVisible(vm == ValidationStyleSheet || vm == ValidationRichText || vm == ValidationMultiLine || vm == ValidationURL);
}

void TextEditor::setText(const QString &text)
{
    if (m_iconThemeModeEnabled)
        m_themeEditor->setTheme(text);
    else
        m_editor->setText(text);
}

void TextEditor::buttonClicked()
{
    const QString oldText = m_editor->text();
    QString newText;
    switch (textPropertyValidationMode()) {
    case ValidationStyleSheet: {
        StyleSheetEditorDialog dlg(m_core, this);
        dlg.setText(oldText);
        if (dlg.exec() != QDialog::Accepted)
            return;
        newText = dlg.text();
    }
        break;
    case ValidationRichText: {
        RichTextEditorDialog dlg(m_core, this);
        dlg.setDefaultFont(m_richTextDefaultFont);
        dlg.setText(oldText);
        if (dlg.showDialog() != QDialog::Accepted)
            return;
        newText = dlg.text(Qt::AutoText);
    }
        break;
    case ValidationMultiLine: {
        PlainTextEditorDialog dlg(m_core, this);
        dlg.setDefaultFont(m_richTextDefaultFont);
        dlg.setText(oldText);
        if (dlg.showDialog() != QDialog::Accepted)
            return;
        newText = dlg.text();
    }
        break;
    case ValidationURL:
        if (oldText.isEmpty() || oldText.startsWith("qrc:"_L1))
            resourceActionActivated();
        else
            fileActionActivated();
        return;
    default:
        return;
    }
    if (newText != oldText) {
        m_editor->setText(newText);
        emit textChanged(newText);
    }
}

void TextEditor::resourceActionActivated()
{
    QString oldPath = m_editor->text();
    if (oldPath.startsWith("qrc:"_L1))
        oldPath.remove(0, 4);
    // returns ':/file'
    QString newPath = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(), oldPath, this);
    if (newPath.startsWith(u':'))
         newPath.remove(0, 1);
    if (newPath.isEmpty() || newPath == oldPath)
        return;
    const QString newText = "qrc:"_L1 + newPath;
    m_editor->setText(newText);
    emit textChanged(newText);
}

void TextEditor::fileActionActivated()
{
    QString oldPath = m_editor->text();
    if (oldPath.startsWith("file:"_L1))
        oldPath = oldPath.mid(5);
    const QString newPath = m_core->dialogGui()->getOpenFileName(this, tr("Choose a File"), oldPath);
    if (newPath.isEmpty() || newPath == oldPath)
        return;
    const QString newText = QUrl::fromLocalFile(newPath).toString();
    m_editor->setText(newText);
    emit textChanged(newText);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
