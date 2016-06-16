/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef FEATURETREEMODEL_H
#define FEATURETREEMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include <QHash>
#include <QTextStream>

QT_BEGIN_NAMESPACE

class Feature;
class Node;

uint qHash(const QModelIndex&);

class FeatureTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FeatureTreeModel(QObject *parent = 0);
    ~FeatureTreeModel();

    void clear();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(const Feature *feature) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void addFeature(Feature *feature);
    Feature* getFeature(const QModelIndex &index) const;

    void readConfig(QTextStream &stream);
    void writeConfig(QTextStream &stream) const;

public slots:
    void featureChanged();

private:
    QModelIndex createIndex(int row, int column,
                            const QModelIndex &parent,
                            const Node *feature) const;
    QModelIndex index(const QModelIndex &parent, const Feature *feature) const;
    bool contains(const QString &section, const Feature *f) const;
    Node* find(const QString &section, const Feature *f) const;
    QStringList findDisabled(const QModelIndex &parent) const;

    QMap<QString, QList<Node*> > sections;
    mutable QHash<QModelIndex, QModelIndex> parentMap;
    mutable QHash<const Feature*, QModelIndex> featureIndexMap;
};

QT_END_NAMESPACE

#endif // FEATURETREEMODEL_H
