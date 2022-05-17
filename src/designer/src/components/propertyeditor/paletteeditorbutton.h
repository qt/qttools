// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PALETTEEDITORBUTTON_H
#define PALETTEEDITORBUTTON_H

#include "propertyeditor_global.h"

#include <QtGui/qpalette.h>
#include <QtWidgets/qtoolbutton.h>

#include "abstractformeditor.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_PROPERTYEDITOR_EXPORT PaletteEditorButton: public QToolButton
{
    Q_OBJECT
public:
    PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette, QWidget *parent = nullptr);
    ~PaletteEditorButton() override;

    void setSuperPalette(const QPalette &palette);
    inline QPalette palette() const
    { return m_palette; }

signals:
    void paletteChanged(const QPalette &palette);

public slots:
    void setPalette(const QPalette &palette);

private slots:
    void showPaletteEditor();

private:
    QPalette m_palette;
    QPalette m_superPalette;
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PALETTEEDITORBUTTON_H
