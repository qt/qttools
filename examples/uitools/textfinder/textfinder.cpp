// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "textfinder.h"

#include <QUiLoader>

#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include <QFile>
#include <QTextStream>

using namespace Qt::StringLiterals;

//! [4]
static QWidget *loadUiFile(QWidget *parent)
{
    QFile file(u":/forms/textfinder.ui"_s);
    file.open(QIODevice::ReadOnly);

    QUiLoader loader;
    return loader.load(&file, parent);
}
//! [4]

//! [5]
static QString loadTextFile()
{
    QFile inputFile(u":/forms/input.txt"_s);
    inputFile.open(QIODevice::ReadOnly);
    QTextStream in(&inputFile);
    return in.readAll();
}
//! [5]

//! [0]
TextFinder::TextFinder(QWidget *parent)
    : QWidget(parent)
{
    QWidget *formWidget = loadUiFile(this);

//! [1]
    ui_findButton = findChild<QPushButton*>("findButton");
    ui_textEdit = findChild<QTextEdit*>("textEdit");
    ui_lineEdit = findChild<QLineEdit*>("lineEdit");
//! [0] //! [1]

//! [2]
    QMetaObject::connectSlotsByName(this);
//! [2]

//! [3a]
    ui_textEdit->setText(loadTextFile());
//! [3a]

//! [3b]
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(formWidget);
//! [3b]

//! [3c]
    setWindowTitle(tr("Text Finder"));
}
//! [3c]

//! [6] //! [7]
void TextFinder::on_findButton_clicked()
{
    QString searchString = ui_lineEdit->text();
    QTextDocument *document = ui_textEdit->document();

    bool found = false;

    // undo previous change (if any)
    document->undo();

    if (searchString.isEmpty()) {
        QMessageBox::information(this, tr("Empty Search Field"),
                                 tr("The search field is empty. "
                                    "Please enter a word and click Find."));
    } else {
        QTextCursor highlightCursor(document);
        QTextCursor cursor(document);

        cursor.beginEditBlock();
//! [6]

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setForeground(Qt::red);

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document->find(searchString, highlightCursor,
                                             QTextDocument::FindWholeWords);

            if (!highlightCursor.isNull()) {
                found = true;
                highlightCursor.movePosition(QTextCursor::WordRight,
                                             QTextCursor::KeepAnchor);
                highlightCursor.mergeCharFormat(colorFormat);
            }
        }

//! [8]
        cursor.endEditBlock();
//! [7] //! [9]

        if (found == false) {
            QMessageBox::information(this, tr("Word Not Found"),
                                     tr("Sorry, the word cannot be found."));
        }
    }
}
//! [8] //! [9]
