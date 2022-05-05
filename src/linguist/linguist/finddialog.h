/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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
    FindDialog(QWidget *parent = 0);
    QRegularExpression &getRegExp() { return m_regExp; }

signals:
    void findNext(const QString& text, DataModel::FindLocation where,
                  bool matchCase, bool ignoreAccelerators, bool skipObsolete, bool useRegExp);

public slots:
    void find();

private slots:
    void emitFindNext();
    void verify();

private:
    QRegularExpression m_regExp;
    bool m_redText = false;
};

QT_END_NAMESPACE

#endif
