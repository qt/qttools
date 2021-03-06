/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "finddialog.h"
//#include "mainwindow.h"
//#include "tabbedbrowser.h"
//#include "helpwindow.h"

#include <QtWidgets/QTextBrowser>
#include <QTextCursor>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLineEdit>
#include <QDateTime>
#include <QtWidgets/QGridLayout>

CaseSensitiveModel::CaseSensitiveModel(int rows, int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
{}
QModelIndexList CaseSensitiveModel::match(const QModelIndex &start, int role, const QVariant &value,
                                          int hits, Qt::MatchFlags flags) const
{
    if (flags == Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap))
        flags |= Qt::MatchCaseSensitive;

    return QStandardItemModel::match(start, role, value, hits, flags);
}

FindDialog::FindDialog(QMainWindow *parent)
    : QDialog(parent)
{
    auto contentsWidget = new QWidget(this);
    //ui.setupUi(contentsWidget);
    //ui.comboFind->setModel(new CaseSensitiveModel(0, 1, ui.comboFind));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(QMargins());
    l->setSpacing(0);
    l->addWidget(contentsWidget);

    auto lastBrowser = 0;
    auto onceFound = false;
    //findExpr.clear();

    auto sb = new QStatusBar(this);
    l->addWidget(sb);


    // Move it to another line and change the text,
    // then lupdate should add this one as a new one, and mark the old one as obsolete.
    sb->showMessage(tr("Enter the text you want to find."));

    //connect(ui.findButton, SIGNAL(clicked()), this, SLOT(findButtonClicked()));
    //connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(reject()));
}

//FindDialog::~FindDialog()
//{
//}

//void FindDialog::findButtonClicked()
//{
  //doFind(ui.radioForward->isChecked());
//}

void FindDialog::doFind(bool forward)
{
  QTextBrowser *browser;// = static_cast<QTextBrowser*>(mainWindow()->browsers()->currentBrowser());
    //sb->clearMessage();

    //if (ui.comboFind->currentText() != findExpr || lastBrowser != browser)
    //    onceFound = false;
    //findExpr = ui.comboFind->currentText();

    //QTextDocument::FindFlags flags = 0;

    //if (ui.checkCase->isChecked())
    //    flags |= QTextDocument::FindCaseSensitively;

    //if (ui.checkWords->isChecked())
    //    flags |= QTextDocument::FindWholeWords;

    QTextCursor c = browser->textCursor();
    if (!c.hasSelection()) {
        if (forward)
            c.movePosition(QTextCursor::Start);
        else
            c.movePosition(QTextCursor::End);

        browser->setTextCursor(c);
    }

    QTextDocument::FindFlags options;
    //if (forward == false)
    //    flags |= QTextDocument::FindBackward;
    bool onceFound = true;
    QTextCursor found;// = browser->document()->find(findExpr, c, flags);
    if (found.isNull()) {
        if (onceFound) {
            if (forward)
	      auto a = tr("Search reached end of the document");//statusMessage(tr("Search reached end of the document"));
            else
	      auto aa = tr("Search reached start of the document");//statusMessage(tr("Search reached start of the document"));
        } else {
	  auto aaa = tr( "Text not found" );//statusMessage(tr( "Text not found" ));
        }
    } else {
        browser->setTextCursor(found);
    }
    onceFound |= !found.isNull();
    auto lastBrowser = browser;
}

void FindDialog::hasFindExpression() const
{
    //% "This is some random text"
  qtTrId("keep_id");

      //return !findExpr.isEmpty();
}

