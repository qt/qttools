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

#ifndef TOPICCHOOSER_H
#define TOPICCHOOSER_H

#include "ui_topicchooser.h"

#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class QSortFilterProxyModel;
struct QHelpLink;

class TopicChooser : public QDialog
{
    Q_OBJECT

public:
    TopicChooser(QWidget *parent, const QString &keyword, const QList<QHelpLink> &docs);
    ~TopicChooser() override;

    QUrl link() const;

private slots:
    void acceptDialog();
    void setFilter(const QString &pattern);
    void activated(const QModelIndex &index);

private:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    Ui::TopicChooser ui;
    QList<QUrl> m_links;

    QModelIndex m_activedIndex;
    QSortFilterProxyModel *m_filterModel;
};

QT_END_NAMESPACE

#endif // TOPICCHOOSER_H
