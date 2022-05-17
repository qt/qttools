// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























//#include "finddialog.h" nothing is picked up from there
//#include "mainwindow.h"
//#include "tabbedbrowser.h"
//#include "helpwindow.h"
#include <QtCore>
#include <QtWidgets/QTextBrowser>
#include <QTextCursor>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLineEdit>
#include <QDateTime>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
class FindDialog
{   Q_OBJECT
    FindDialog(QMainWindow *parent);
    void findButtonClicked();
    void doFind(bool forward);
    void statusMessage(const QString &message);
    //QMainWindow *mainWindow() const;
    void reset();


};


FindDialog::FindDialog(QMainWindow *parent)
{
    //auto contentsWidget = new QWidget(this);
    //ui.setupUi(contentsWidget);
    //ui.comboFind->setModel(new CaseSensitiveModel(0, 1, ui.comboFind));

    //QVBoxLayout *l = new QVBoxLayout(this);
    //l->setContentsMargins(QMargins());
    //l->setSpacing(0);
    //l->addWidget(contentsWidget);

    auto lastBrowser = 0;
    auto onceFound = false;
    //findExpr.clear();

    auto sb = new QStatusBar;
    //l->addWidget(sb);

    sb->showMessage(tr("Enter the text you are looking for."));

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
    //QTextBrowser *browser = static_cast<QTextBrowser*>(mainWindow()->browsers()->currentBrowser());
    //sb->clearMessage();

    //if (ui.comboFind->currentText() != findExpr || lastBrowser != browser)
      //  onceFound = false;
    //findExpr = ui.comboFind->currentText();
    bool onceFound = false;
    //QTextDocument::FindFlags flags = 0;
    /*
    if (ui.checkCase->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    if (ui.checkWords->isChecked())
        flags |= QTextDocument::FindWholeWords;

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
    */
    QTextCursor found;//browser->document()->find(findExpr, c, flags);
    if (found.isNull()) {
        if (onceFound) {
            if (forward)
                statusMessage(tr("Search reached end of the document"));
            else
                statusMessage(tr("Search reached start of the document"));
        } else {
            statusMessage(tr( "Text not found" ));
        }
    } else {
        //browser->setTextCursor(found);
    }
    onceFound |= !found.isNull();
    //lastBrowser = browser;
}

//bool FindDialog::hasFindExpression() const
//{
//    return !findExpr.isEmpty();
//}

void FindDialog::statusMessage(const QString &message)
{   /*
    if (isVisible())
        sb->showMessage(message);
    else
        static_cast<MainWindow*>(parent())->statusBar()->showMessage(message, 2000); */
}

//QMainWindow *FindDialog::mainWindow() const
//{
//    return static_cast<MainWindow*>(parentWidget());
//}

void FindDialog::reset()
{
    //ui.comboFind->setFocus();
    //ui.comboFind->lineEdit()->setSelection(
      //  0, ui.comboFind->lineEdit()->text().length());

    QString s = QCoreApplication::translate("QCoreApplication", "with comment", "comment");
    QString s1 = QCoreApplication::translate("QCoreApplication", "empty comment", "");
    QString s2 = QCoreApplication::translate("QCoreApplication", "null comment", 0);
    QString s3 = tr("null comment");

    QString s4 = QCoreApplication::translate("QCoreApplication", "encoding, using QCoreApplication", 0);
    QString s5 = QCoreApplication::translate("QCoreApplication", "encoding, using QApplication", 0);

    QString s6 = QCoreApplication::translate("Kåntekst", "encoding, using QApplication", 0);
    QString s7 = QCoreApplication::translate("QTranslator", "Key", "disambiguation");
}
