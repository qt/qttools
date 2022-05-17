// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
