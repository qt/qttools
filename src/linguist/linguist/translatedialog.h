// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TRANSLATEDIALOG_H
#define TRANSLATEDIALOG_H

#include "ui_translatedialog.h"
#include <QDialog>

QT_BEGIN_NAMESPACE

class TranslateDialog : public QDialog
{
    Q_OBJECT

public:
    enum {
        Skip,
        Translate,
        TranslateAll
    };

    TranslateDialog(QWidget *parent = 0);

    bool markFinished() const { return m_ui.ckMarkFinished->isChecked(); }
    Qt::CaseSensitivity caseSensitivity() const
        { return m_ui.ckMatchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive; }
    QString findText() const { return m_ui.ledFindWhat->text(); }
    QString replaceText() const { return m_ui.ledTranslateTo->text(); }

signals:
    void requestMatchUpdate(bool &hit);
    void activated(int mode);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void emitFindNext();
    void emitTranslateAndFindNext();
    void emitTranslateAll();
    void verifyText();

private:
    Ui::TranslateDialog m_ui;
};


QT_END_NAMESPACE
#endif  //TRANSLATEDIALOG_H

