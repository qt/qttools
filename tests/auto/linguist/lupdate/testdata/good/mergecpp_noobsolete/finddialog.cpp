// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























#include "finddialog.h"




#include <QtWidgets/QTextBrowser>
#include <QTextCursor>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLineEdit>
#include <QDateTime>
#include <QtWidgets/QGridLayout>
//#include <QtWidgets/QDialog>
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

    sb->showMessage(tr("Enter the text you want to find."));

    //connect(ui.findButton, SIGNAL(clicked()), this, SLOT(findButtonClicked()));
    //connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(reject()));
}

//FindDialog::~FindDialog()
//{
//}

//void FindDialog::findButtonClicked()
//{
//    doFind(ui.radioForward->isChecked());
//}

void FindDialog::doFind(bool forward)
{
    QTextBrowser *browser = NULL;// = static_cast<QTextBrowser*>(mainWindow()->browsers()->currentBrowser());
    //sb->clearMessage();

    //if (ui.comboFind->currentText() != findExpr || lastBrowser != browser)
    //  onceFound = false;
    //findExpr = ui.comboFind->currentText();

    QTextDocument::FindFlags flags;// = 0;

    //if (ui.checkCase->isChecked())
    //  flags |= QTextDocument::FindCaseSensitively;

    //if (ui.checkWords->isChecked())
    //  flags |= QTextDocument::FindWholeWords;

    QTextCursor c = browser->textCursor();
    if (!c.hasSelection()) {
        if (forward)
            c.movePosition(QTextCursor::Start);
        else
            c.movePosition(QTextCursor::End);

        browser->setTextCursor(c);
    }

    QTextDocument::FindFlags options;
    if (forward == false)
        flags |= QTextDocument::FindBackward;
    bool onceFound = true;
    QTextCursor found;// = browser->document()->find(findExpr, c, flags);
    if (found.isNull()) {
        if (onceFound) {
            if (forward)
	        auto a = tr("Search reached end of the document");//statusMessage(tr("Search reached end of the document"));
            else
	        auto aa = tr("Search reached start of the document");//statusMessage(tr("Search reached start of the document"));
        } else {
	    auto aaa = tr( "Text not found" ); //statusMessage(tr( "Text not found" ));
        }
    } else {
        browser->setTextCursor(found);
    }
    onceFound |= !found.isNull();
    auto lastBrowser = browser;
}

void FindDialog::hasFindExpression() const
{
    // statusMessage(tr( "Should be obsolete" ));

    //% "This is some random text"
  qtTrId("keep_id");
}
