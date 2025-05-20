// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DIALOG_H
#define DIALOG_H

#include "ui_dialog.h"

#include <QDialog>
#include <memory>

class Dialog final : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

signals:
    void timeZoneSelected(const QString &timeZone);

private:
    std::unique_ptr<Ui::Dialog> m_ui;
};

#endif // DIALOG_H
