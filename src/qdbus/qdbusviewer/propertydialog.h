// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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

