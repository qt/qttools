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
******************************************************************************/

#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QTableWidget>
#include <QDialogButtonBox>

class PropertyDialog: public QDialog
{
    Q_OBJECT
public:
    explicit PropertyDialog(QWidget *parent = 0, Qt::WindowFlags f = {});

    void addProperty(const QString &name, int type);
    void setInfo(const QString &caption);

    QList<QVariant> values() const;

    int exec() override;

private:
    QLabel *label;
    QTableWidget *propertyTable;
    QDialogButtonBox *buttonBox;
};

#endif

