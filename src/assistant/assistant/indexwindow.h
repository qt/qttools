// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INDEXWINDOW_H
#define INDEXWINDOW_H

#include <QtCore/QUrl>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class QHelpIndexWidget;
class QModelIndex;
struct QHelpLink;

class IndexWindow : public QWidget
{
    Q_OBJECT
    Q_MOC_INCLUDE(<QtHelp/qhelplink.h>)

public:
    IndexWindow(QWidget *parent = nullptr);
    ~IndexWindow() override;

    void setSearchLineEditText(const QString &text);
    QString searchLineEditText() const
    {
        return m_searchLineEdit->text();
    }

signals:
    void linkActivated(const QUrl &link);
    void documentsActivated(const QList<QHelpLink> &documents, const QString &keyword);
    void escapePressed();

private slots:
    void filterIndices(const QString &filter);
    void enableSearchLineEdit();
    void disableSearchLineEdit();

private:
    bool eventFilter(QObject *obj, QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void open(QHelpIndexWidget *indexWidget, const QModelIndex &index);

    QLineEdit *m_searchLineEdit;
    QHelpIndexWidget *m_indexWidget;
};

QT_END_NAMESPACE

#endif // INDEXWINDOW_H
