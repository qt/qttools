// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "paletteeditorbutton.h"
#include "paletteeditor.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

PaletteEditorButton::PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette, QWidget *parent)
    : QToolButton(parent),
      m_palette(palette)
{
    m_core = core;
    setFocusPolicy(Qt::NoFocus);
    setText(tr("Change Palette"));
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    connect(this, &QAbstractButton::clicked, this, &PaletteEditorButton::showPaletteEditor);
}

PaletteEditorButton::~PaletteEditorButton() = default;

void PaletteEditorButton::setPalette(const QPalette &palette)
{
    m_palette = palette;
}

void PaletteEditorButton::setSuperPalette(const QPalette &palette)
{
    m_superPalette = palette;
}

void PaletteEditorButton::showPaletteEditor()
{
    int result;
    QPalette pal = PaletteEditor::getPalette(m_core, nullptr, m_palette, m_superPalette, &result);
    if (result == QDialog::Accepted) {
        m_palette = pal;
        emit paletteChanged(m_palette);
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
