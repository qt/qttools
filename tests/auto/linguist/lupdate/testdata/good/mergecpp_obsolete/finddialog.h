// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























#include <QtWidgets/QTextBrowser>
#include <QTextCursor>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QLineEdit>
#include <QDateTime>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialog>
#include <QtGui>
#include <QtWidgets/QMainWindow>

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog(QWidget *parent = 0);
    FindDialog(QMainWindow *parent = 0);
    void doFind(bool forward);
    void hasFindExpression() const;

signals:

public slots:
    void find();

private slots:
    void emitFindNext();
    void verify();

private:
    bool m_redText = false;
};


class CaseSensitiveModel: public QStandardItemModel
{
  CaseSensitiveModel(int rows, int columns, QObject *parent);
  QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const;
};
