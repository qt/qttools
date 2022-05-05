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

#include "paletteeditorbutton.h"
#include "paletteeditor.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

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

QT_END_NAMESPACE
