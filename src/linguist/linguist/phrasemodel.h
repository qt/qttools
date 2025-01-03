// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PHRASEMODEL_H
#define PHRASEMODEL_H

#include "phrase.h"

#include <QList>
#include <QAbstractItemModel>

QT_BEGIN_NAMESPACE

class PhraseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PhraseModel(QObject *parent = 0)
        : QAbstractTableModel(parent)
    {}

    void removePhrases();
    QList<Phrase *> phraseList() const {return plist;}

    QModelIndex addPhrase(Phrase *p);
    void removePhrase(const QModelIndex &index);

    Phrase *phrase(const QModelIndex &index) const;
    void setPhrase(const QModelIndex &indx, Phrase *ph);
    QModelIndex index(Phrase * const phr) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    { return QAbstractTableModel::index(row, column, parent); }

    // from qabstracttablemodel
    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    // HACK: This model will be displayed in a _TreeView_
    // which has a tendency to expand 'children' on double click
    bool hasChildren(const QModelIndex &parent) const override
    { return !parent.isValid(); }

private:
    QList<Phrase *> plist;
};

QT_END_NAMESPACE

#endif // PHRASEMODEL_H
