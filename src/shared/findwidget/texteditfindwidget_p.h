// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of Qt Designer. This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#ifndef TEXTEDITFINDWIDGET_H
#define TEXTEDITFINDWIDGET_H

#include "abstractfindwidget_p.h"

QT_BEGIN_NAMESPACE

class QTextEdit;

class TextEditFindWidget : public AbstractFindWidget
{
    Q_OBJECT

public:
    explicit TextEditFindWidget(FindFlags flags = FindFlags(), QWidget *parent = 0);

    QTextEdit *textEdit() const
    { return m_textEdit; }

    void setTextEdit(QTextEdit *textEdit);

protected:
    void deactivate() override;
    void find(const QString &textToFind, bool skipCurrent,
              bool backward, bool *found, bool *wrapped) override;

private:
    QTextEdit *m_textEdit;
};

QT_END_NAMESPACE

#endif  // TEXTEDITFINDWIDGET_H
