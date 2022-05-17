// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "ui_finddialog.h"
#include "messagemodel.h"

#include <QDialog>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE

class FindDialog : public QDialog, public Ui::FindDialog
{
    Q_OBJECT
public:
    enum FindOption {
        MatchCase = 0x1,
        IgnoreAccelerators = 0x2,
        SkipObsolete = 0x4,
        UseRegExp = 0x8
    };
    Q_DECLARE_FLAGS(FindOptions, FindOption)
    FindDialog(QWidget *parent = 0);
    QRegularExpression &getRegExp() { return m_regExp; }

signals:
    void findNext(const QString& text, DataModel::FindLocation where,
                  FindDialog::FindOptions options, int statusFilter);

public slots:
    void find();

private slots:
    void emitFindNext();
    void verify();
    void statusFilterChanged();

private:
    QRegularExpression m_regExp;
    bool m_redText = false;
    int m_lastStateFilter = -1;
    bool m_storedSkipObsolete = false;
};

QT_END_NAMESPACE

#endif
