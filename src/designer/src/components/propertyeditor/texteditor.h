// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <shared_enums_p.h>
#include <textpropertyeditor_p.h>

#include <QtWidgets/qwidget.h>

#include <QtGui/qfont.h>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QToolButton)

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal
{

class IconThemeEditor;

class TextEditor : public QWidget
{
    Q_OBJECT
public:
    TextEditor(QDesignerFormEditorInterface *core, QWidget *parent);

    TextPropertyValidationMode textPropertyValidationMode() const;
    void setTextPropertyValidationMode(TextPropertyValidationMode vm);

    void setRichTextDefaultFont(const QFont &font) { m_richTextDefaultFont = font; }
    QFont richTextDefaultFont() const { return m_richTextDefaultFont; }

    void setSpacing(int spacing);

    TextPropertyEditor::UpdateMode updateMode() const { return m_editor->updateMode(); }
    void setUpdateMode(TextPropertyEditor::UpdateMode um) { m_editor->setUpdateMode(um); }

    void setIconThemeModeEnabled(bool enable);

public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);

private slots:
    void buttonClicked();
    void resourceActionActivated();
    void fileActionActivated();

private:
    TextPropertyEditor *m_editor;
    IconThemeEditor *m_themeEditor;
    bool m_iconThemeModeEnabled;
    QFont m_richTextDefaultFont;
    QToolButton *m_button;
    QMenu *m_menu;
    QAction *m_resourceAction;
    QAction *m_fileAction;
    QHBoxLayout *m_layout;
    QDesignerFormEditorInterface *m_core;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif //TEXTEDITOR_H
