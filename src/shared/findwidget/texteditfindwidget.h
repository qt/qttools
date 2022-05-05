/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef TEXTEDITFINDWIDGET_H
#define TEXTEDITFINDWIDGET_H

#include "abstractfindwidget.h"

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
