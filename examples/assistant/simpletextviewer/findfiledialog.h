// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FINDFILEDIALOG_H
#define FINDFILEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QToolButton;
class QTreeWidget;
QT_END_NAMESPACE

class Assistant;
class TextEdit;

//! [0]
class FindFileDialog : public QDialog
{
    Q_OBJECT

public:
    FindFileDialog(TextEdit *editor, Assistant *assistant);

private slots:
    void browse();
    void help();
    void openFile();
    void update();

private:
    void findFiles();
    void showFiles(const QStringList &files);

    void createButtons();
    void createComboBoxes();
    void createFilesTree();
    void createLabels();
    void createLayout();

    TextEdit *currentEditor;
    Assistant *currentAssistant;
    QTreeWidget *foundFilesTree;

    QComboBox *directoryComboBox;
    QComboBox *fileNameComboBox;

    QLabel *directoryLabel;
    QLabel *fileNameLabel;

    QDialogButtonBox *buttonBox;

    QToolButton *browseButton;
};
//! [0]

#endif
