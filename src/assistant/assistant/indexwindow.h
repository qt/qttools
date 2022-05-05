/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
******************************************************************************/

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
