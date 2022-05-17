// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OPENPAGESMODEL_H
#define OPENPAGESMODEL_H

#include "openpagesmanager.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class HelpViewer;
class QUrl;

class OpenPagesModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class OpenPagesManager;
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    HelpViewer *addPage(const QUrl &url, qreal zoom = 0);
    void removePage(int index);
    HelpViewer *pageAt(int index) const;

private slots:
    void handleTitleChanged();

private:
    OpenPagesModel(QObject *parent);

private:
    QList<HelpViewer *> m_pages;
};

QT_END_NAMESPACE

#endif // OPENPAGESMODEL_H
