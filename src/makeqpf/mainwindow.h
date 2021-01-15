/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "qpf2.h"

QT_BEGIN_NAMESPACE

class QListWidgetItem;

class MainWindow : public QMainWindow, Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow(const QString &customFont);

private Q_SLOTS:
    void on_actionAdd_Custom_Font_triggered();
    void on_selectAll_clicked();
    void on_deselectAll_clicked();
    void on_invertSelection_clicked();
    void fontChanged();
    void on_browsePath_clicked();
    void on_browseSampleFile_clicked();
    void on_generate_clicked();
    void on_sampleFile_editingFinished();

private:
    void populateCharacterRanges();
    void addCustomFont(const QString &fontFile);

private:
    QList<QPF::CharacterRange> sampleFileRanges;
};

QT_END_NAMESPACE

#endif // MAINWINDOW_H
