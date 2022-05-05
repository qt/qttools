/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Assistant module of the Qt Toolkit.
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
