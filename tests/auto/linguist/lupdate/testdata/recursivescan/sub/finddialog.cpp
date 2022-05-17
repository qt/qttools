// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























#include "finddialog.h"
#include "mainwindow.h"
#include "tabbedbrowser.h"
#include "helpwindow.h"

#include <QTextBrowser>
#include <QTextCursor>
#include <QStatusBar>
#include <QLineEdit>
#include <QDateTime>
#include <QGridLayout>

FindDialog::FindDialog(MainWindow *parent)
    : QDialog(parent)
{
    sb->showMessage(tr("Enter the text you want to find."));
}

void FindDialog::doFind(bool forward)
{
    QTextCursor found = browser->document()->find(findExpr, c, flags);
    if (found.isNull()) {
        if (onceFound) {
            if (forward)
                statusMessage(tr("Search reached end of the document"));
            else
                statusMessage(tr("Search reached start of the document"));
        } else {
            statusMessage(tr( "Text not found" ));
        }
    }
}
